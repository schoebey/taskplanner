#include "addsubtaskcommand.h"

#include "taskinterface.h"
#include "manager.h"

#include "taskwidget.h"
#include "groupwidget.h"
#include "widgetmanager.h"

AddSubTaskCommand::AddSubTaskCommand(task_id parentTaskId,
                                     task_id childTaskId,
                                     Manager* pManager,
                                     WidgetManager* pWidgetManager)
  : m_parentTaskId(parentTaskId),
    m_prevParentTaskId(-1),
    m_iPrevPos(-1),
    m_childTaskId(childTaskId),
    m_pManager(pManager),
    m_pWidgetManager(pWidgetManager)
{
  setText(QString("Add sub task"));
}

void AddSubTaskCommand::undo()
{
  ITask* pTask = m_pManager->task(m_parentTaskId);
  if (nullptr != pTask)
  {
    if (pTask->removeTask(m_childTaskId))
    {
      auto pTaskWidget = m_pWidgetManager->taskWidget(m_parentTaskId);
      auto pChildTaskWidget = m_pWidgetManager->taskWidget(m_childTaskId);
      if (nullptr != pTaskWidget)
      {
        pTaskWidget->removeTask(pChildTaskWidget);

        pTaskWidget = m_pWidgetManager->taskWidget(m_prevParentTaskId);
        if (nullptr != pTaskWidget)
        {
          pTaskWidget->addTask(pChildTaskWidget);
        }
        else
        {
          auto pGroupWidget = m_pWidgetManager->groupWidget(pTask->group());
          if (nullptr != pGroupWidget)
          {
            pGroupWidget->insertTask(pChildTaskWidget, m_iPrevPos);
          }
        }
      }
    }
  }
}

void AddSubTaskCommand::redo()
{
  ITask* pTask = m_pManager->task(m_parentTaskId);
  ITask* pChildTask = m_pManager->task(m_childTaskId);
  if (nullptr != pTask && nullptr != pChildTask)
  {
    bool bOk(false);
    m_iPrevPos = pChildTask->propertyValue("sort_priority").toInt(&bOk);
    if (!bOk)  { m_iPrevPos = -1; }

    m_prevParentTaskId = pTask->parentTask();
    if (pTask->addTask(m_childTaskId))
    {
      auto pTaskWidget = m_pWidgetManager->taskWidget(m_parentTaskId);
      auto pChildTaskWidget = m_pWidgetManager->taskWidget(m_childTaskId);
      if (nullptr != pTaskWidget)
      {
        auto pCurrentGroupWidget = pChildTaskWidget->groupWidget();
        if (nullptr != pCurrentGroupWidget)
        {
          pCurrentGroupWidget->removeTask(pChildTaskWidget);
        }
        pTaskWidget->addTask(pChildTaskWidget);
      }
    }
  }
}
