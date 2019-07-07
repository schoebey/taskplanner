#include "addsubtaskcommand.h"
#include "manager.h"
#include "widgetmanager.h"
#include "taskinterface.h"
#include "taskwidget.h"

#include <cassert>

AddSubTaskCommand::AddSubTaskCommand(task_id parentTaskId,
                               const QString& sName,
                               const QString& sDescription,
                               Manager* pManager,
                               WidgetManager* pWidgetManager)
  : m_taskId(-1),
    m_parentTaskId(parentTaskId),
    m_pManager(pManager),
    m_pWidgetManager(pWidgetManager)
{
  setText(QString("add subtask \"%1\"").arg(sName));

  m_properties["name"] = sName;
  m_properties["description"] = sDescription;
}

void AddSubTaskCommand::undo()
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
  pParentTaskWidget->removeTask(m_pWidgetManager->taskWidget(m_taskId));
  m_pWidgetManager->deleteTaskWidget(m_taskId);
  m_pManager->removeTask(m_taskId);
}

void AddSubTaskCommand::redo()
{
  ITask* pParentTask = m_pManager->task(m_parentTaskId);
  TaskWidget* pParentTaskWidget = m_pWidgetManager->taskWidget(m_parentTaskId);
  if (nullptr != pParentTask &&
      nullptr != pParentTaskWidget)
  {
    ITask* pTask = m_pManager->addTask(m_taskId);
    m_taskId = pTask->id();

    for (const auto& el : m_properties)
    {
      bool bAccepted = pTask->setPropertyValue(el.first, el.second);
      assert(bAccepted);
    }

    pTask->setGroup(pParentTask->group());
    pParentTask->addTask(pTask->id());
    auto pTaskWidget = m_pWidgetManager->createTaskWidget(pTask->id());

    int iPos = -1;
    auto it = m_properties.find("sort_priority");
    if (it != m_properties.end())
    {
      bool bOk(false);
      iPos = it->second.toInt(&bOk);
      if (!bOk) { iPos = -1; }
    }
    pParentTaskWidget->insertTask(pTaskWidget/*, iPos*/);
    pTaskWidget->show();
  }
}
