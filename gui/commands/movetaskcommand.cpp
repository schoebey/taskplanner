#include "movetaskcommand.h"
#include "taskinterface.h"
#include "groupinterface.h"
#include "manager.h"
#include "taskwidget.h"
#include "groupwidget.h"
#include "widgetmanager.h"

namespace
{
  void moveTask(ITask* pTask,
                TaskWidget* pTaskWidget,
                IGroup* pOldGroup,
                GroupWidget* pOldGroupWidget,
                IGroup* pNewGroup,
                GroupWidget* pNewGroupWidget,
                int iNewPosition,
                Manager* pManager)
  {
    if (nullptr == pTask || nullptr == pTaskWidget ||
        nullptr == pOldGroup || nullptr == pOldGroupWidget ||
        nullptr == pNewGroup || nullptr == pNewGroupWidget)
    {
      return;
    }

    pTask->setGroup(pNewGroup->id());

    // if the task has moved groups, fill the priority gaps in the old group
    // by building a sequence, determining the jumps and correcting the priorities
    // of following tasks.
    if (pOldGroup != pNewGroup)
    {
      if (nullptr != pOldGroup)
      {
        std::map<int, ITask*> tasksByPriority;
        for (const auto& taskId : pOldGroup->taskIds())
        {
          ITask* pOtherTask = pManager->task(taskId);
          if (nullptr != pOtherTask)
          {
            tasksByPriority[pOtherTask->priority().priority(0)] = pOtherTask;
          }
        }

        // determine the gaps
        int iPrevPrio = -1;
        int iDelta = 0;
        for (const auto& el : tasksByPriority)
        {
          SPriority prio = el.second->priority();
          iDelta += el.first - iPrevPrio - 1;
          iPrevPrio = prio.priority(0);
          if (0 < iDelta)
          {
            prio.setPriority(0, iPrevPrio - iDelta);
            el.second->setPriority(prio);
          }
        }
      }
    }

    SPriority prio = pTask->priority();

    if (nullptr != pNewGroup)
    {
      // increment the priority of every task that is below the moved task, by one.
      for (const auto& taskId : pNewGroup->taskIds())
      {
        ITask* pOtherTask = pManager->task(taskId);
        if (nullptr != pOtherTask)
        {
          // if the item's priority lies between the old and the new priority, increment it by one
          SPriority otherPrio = pOtherTask->priority();
          if (iNewPosition <= otherPrio.priority(0)  &&
              prio.priority(0) >= otherPrio.priority(0))
          {
            otherPrio.setPriority(0, otherPrio.priority(0) + 1);
          }
          else if (iNewPosition >= otherPrio.priority(0)  &&
                   prio.priority(0) < otherPrio.priority(0))
          {
            otherPrio.setPriority(0, otherPrio.priority(0) - 1);
          }

          pOtherTask->setPriority(otherPrio);
        }
      }
    }


    prio.setPriority(0, iNewPosition);
    pTask->setPriority(prio);


    if (nullptr != pOldGroupWidget)
    {
      pOldGroupWidget->removeTask(pTaskWidget);
    }
    if (nullptr != pNewGroupWidget)
    {
      pNewGroupWidget->insertTask(pTaskWidget, iNewPosition);
    }
  }
}

MoveTaskCommand::MoveTaskCommand(task_id taskId,
                                 group_id oldGroupId,
                                 group_id newGroupId,
                                 int iOldPosition,
                                 int iNewPosition,
                                 Manager* pManager,
                                 WidgetManager* pWidgetManager)
  : m_taskId(taskId),
    m_oldGroupId(oldGroupId),
    m_newGroupId(newGroupId),
    m_iOldPosition(iOldPosition),
    m_iNewPosition(iNewPosition),
    m_pManager(pManager),
    m_pWidgetManager(pWidgetManager)
{
  setText(QString("move task"));
}

MoveTaskCommand::~MoveTaskCommand()
{

}

void MoveTaskCommand::undo()
{
  moveTask(m_pManager->task(m_taskId), m_pWidgetManager->taskWidget(m_taskId),
           m_pManager->group(m_newGroupId), m_pWidgetManager->groupWidget(m_newGroupId),
           m_pManager->group(m_oldGroupId), m_pWidgetManager->groupWidget(m_oldGroupId),
           m_iOldPosition, m_pManager);
}

void MoveTaskCommand::redo()
{
  moveTask(m_pManager->task(m_taskId), m_pWidgetManager->taskWidget(m_taskId),
           m_pManager->group(m_oldGroupId), m_pWidgetManager->groupWidget(m_oldGroupId),
           m_pManager->group(m_newGroupId), m_pWidgetManager->groupWidget(m_newGroupId),
           m_iNewPosition, m_pManager);
}
