#include "addtaskcommand.h"
#include "groupwidget.h"
#include "manager.h"
#include "widgetmanager.h"
#include "taskinterface.h"
#include "groupinterface.h"
#include "taskwidget.h"

#include <cassert>

AddTaskCommand::AddTaskCommand(group_id groupId,
                               task_id parentTaskId,
                               const QString& sName,
                               const QString& sDescription,
                               Manager* pManager,
                               WidgetManager* pWidgetManager)
  : m_taskId(-1),
    m_groupId(groupId),
    m_parentTaskId(parentTaskId),
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


  TaskWidget* pParentTaskWidget = m_pWidgetManager->taskWidget(m_parentTaskId);
  GroupWidget* pGroupWidget = m_pWidgetManager->groupWidget(m_groupId);
  if (nullptr != pParentTaskWidget)
  {
    pParentTaskWidget->removeTask(m_pWidgetManager->taskWidget(m_taskId));
  }
  else if (nullptr != pGroupWidget)
  {
    pGroupWidget->removeTask(m_pWidgetManager->taskWidget(m_taskId));
  }
  m_pWidgetManager->deleteTaskWidget(m_taskId);
  m_pManager->removeTask(m_taskId);
}

void AddTaskCommand::redo()
{
  IGroup* pGroup = m_pManager->group(m_groupId);
  GroupWidget* pGroupWidget = m_pWidgetManager->groupWidget(m_groupId);

  ITask* pParentTask = m_pManager->task(m_parentTaskId);
  auto pParentTaskWidget = m_pWidgetManager->taskWidget(m_parentTaskId);

  if ((nullptr != pGroup && nullptr != pGroupWidget) ||
      (nullptr != pParentTask && nullptr != pParentTaskWidget))
  {
    ITask* pTask = m_pManager->addTask(m_taskId);
    m_taskId = pTask->id();

    for (const auto& el : m_properties)
    {
      bool bAccepted = pTask->setPropertyValue(el.first, el.second);
      assert(bAccepted);
      Q_UNUSED(bAccepted)
    }

    if (nullptr != pParentTask)
    {
      pParentTask->addTask(m_taskId);
      pTask->setGroup(pParentTask->group());
    }
    else if (nullptr != pGroup)
    {
      pGroup->addTask(pTask->id());
      pTask->setGroup(m_groupId);
    }


    auto pTaskWidget = m_pWidgetManager->createTaskWidget(pTask->id());

    int iPos = -1;
    auto it = m_properties.find("sort_priority");
    if (it != m_properties.end())
    {
      bool bOk(false);
      iPos = it->second.toInt(&bOk);
      if (!bOk) { iPos = -1; }
    }

    if (nullptr != pParentTaskWidget)
    {
      pParentTaskWidget->insertTask(pTaskWidget, iPos);
    }
    else if (nullptr != pGroupWidget)
    {
      pGroupWidget->insertTask(pTaskWidget, iPos);
    }

    pTaskWidget->setFocus();
    pTaskWidget->show();
  }
}
