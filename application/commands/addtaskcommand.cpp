#include "addtaskcommand.h"
#include "groupwidget.h"
#include "manager.h"
#include "widgetmanager.h"
#include "taskinterface.h"
#include "groupinterface.h"
#include "taskwidget.h"

#include <cassert>

AddTaskCommand::AddTaskCommand(group_id groupId,
                               const QString& sName,
                               const QString& sDescription,
                               Manager* pManager,
                               WidgetManager* pWidgetManager)
  : m_taskId(-1),
    m_groupId(groupId),
    m_pManager(pManager),
    m_pWidgetManager(pWidgetManager)
{
  setText(QString("add task \"%1\"").arg(sName));

  m_properties["name"] = sName;
  m_properties["description"] = sDescription;
}

void AddTaskCommand::undo()
{
  ITask* pTask = m_pManager->task(m_taskId);
  if (nullptr != pTask)
  {
    for (const auto& sName : pTask->propertyNames())
    {
      m_properties[sName] = pTask->propertyValue(sName);
    }
  }

  GroupWidget* pGroupWidget = m_pWidgetManager->groupWidget(m_groupId);
  pGroupWidget->removeTask(m_pWidgetManager->taskWidget(m_taskId));
  m_pWidgetManager->deleteTaskWidget(m_taskId);
  m_pManager->removeTask(m_taskId);
}

void AddTaskCommand::redo()
{
  IGroup* pGroup = m_pManager->group(m_groupId);
  GroupWidget* pGroupWidget = m_pWidgetManager->groupWidget(m_groupId);
  if (nullptr != pGroup &&
      nullptr != pGroupWidget)
  {
    ITask* pTask = m_pManager->addTask(m_taskId);
    m_taskId = pTask->id();

    for (const auto& el : m_properties)
    {
      bool bAccepted = pTask->setPropertyValue(el.first, el.second);
      assert(bAccepted);
    }

    pGroup->addTask(pTask->id());
    auto pTaskWidget = m_pWidgetManager->createTaskWidget(pTask->id());

    int iPos = -1;
    auto it = m_properties.find("sort_priority");
    if (it != m_properties.end())
    {
      bool bOk(false);
      iPos = it->second.toInt(&bOk);
      if (!bOk) { iPos = -1; }
    }
    pGroupWidget->insertTask(pTaskWidget, iPos);
    pTaskWidget->show();
  }
}
