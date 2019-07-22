#include "movetaskcommand.h"
#include "taskinterface.h"
#include "groupinterface.h"
#include "manager.h"
#include "taskwidget.h"
#include "groupwidget.h"
#include "tasklistwidget.h"
#include "widgetmanager.h"
#include "itaskcontainerwidget.h"

#include <QDebug>

namespace
{
  template<typename T> void fillSortPriorityGapsOfChildren(T* pT, task_id parentTaskId, Manager* pManager)
  {
    std::map<int, ITask*> tasksByPriority;
    for (const auto& taskId : pT->taskIds())
    {
      ITask* pOtherTask = pManager->task(taskId);
      if (nullptr != pOtherTask && parentTaskId == pOtherTask->parentTask())
      {
        tasksByPriority[pOtherTask->priority().priority(0)] = pOtherTask;
      }
    }

    int iPrevPriority = 0;
    for (auto& el : tasksByPriority)
    {
      int iDelta = el.first - iPrevPriority - 1;
      if (iDelta > 0)
      {
        auto prio = el.second->priority();
        prio.setPriority(0, prio.priority(0) - iDelta);
        el.second->setPriority(prio);
      }
      iPrevPriority = el.second->priority().priority(0);
    }
  }

  template<typename T> void correctSortPriorityOfChildren(T* pT, task_id parentTaskId, int iStartPosition, int iEndPosition, int iDelta, Manager* pManager)
  {
    for (const auto& taskId : pT->taskIds())
    {
      ITask* pOtherTask = pManager->task(taskId);
      if (nullptr != pOtherTask && parentTaskId == pOtherTask->parentTask())
      {
        auto prio = pOtherTask->priority();
        if (prio.priority(0) >= iStartPosition && prio.priority(0) < iEndPosition)
        {
          prio.setPriority(0, prio.priority(0) + iDelta);
          pOtherTask->setPriority(prio);
        }
      }
    }
  }

  void moveTask(ITask* pTask,
                TaskWidget* pTaskWidget,
                IGroup* pOldGroup,
                ITaskContainerWidget* pOldGroupWidget,
                IGroup* pNewGroup,
                ITaskContainerWidget* pNewGroupWidget,
                ITask* pOldParentTask,
                ITaskContainerWidget* pOldParentTaskWidget,
                ITask* pNewParentTask,
                ITaskContainerWidget* pNewParentTaskWidget,
                int iNewPosition,
                Manager* pManager)
  {
    if (nullptr == pTask || nullptr == pTaskWidget)
    {
      return;
    }


    int iCurrentPosition = pTask->priority().priority(0);



    /* item could be moved
     *  * from a group
     *    * to a new position within the same group
     *    * to a new group
     *    * to a new task
     *  * from a task
     *    * to a new position within the same task
     *    * to a new group
     *    * to a new task
     */
    if (nullptr != pOldParentTask)
    {
      if (pOldParentTask == pNewParentTask)
      {
        // order within the task has changed
        int iDelta = iNewPosition - iCurrentPosition;
        if (iDelta < 0)
        {
          correctSortPriorityOfChildren<ITask>(pOldParentTask, pOldParentTask->id(), iNewPosition, iCurrentPosition, 1, pManager);
        }
        else if (iDelta > 0)
        {
          correctSortPriorityOfChildren<ITask>(pOldParentTask, pOldParentTask->id(), iCurrentPosition, iNewPosition, -1, pManager);
        }
      }
      else if (nullptr != pNewParentTask)
      {
        // task has been moved to a new parent task
        pTask->setGroup(pNewParentTask->group());
        pTask->setParentTask(pNewParentTask->id());
        fillSortPriorityGapsOfChildren<ITask>(pOldParentTask, pOldParentTask->id(), pManager);
        correctSortPriorityOfChildren<ITask>(pNewParentTask, pNewParentTask->id(), iNewPosition, std::numeric_limits<int>::max(), 1, pManager);
      }
      else if (nullptr != pNewGroup)
      {
        // task has been removed from a parent task and moved to a group
        pTask->setGroup(pNewGroup->id());
        pTask->setParentTask(-1);
        fillSortPriorityGapsOfChildren<ITask>(pOldParentTask, pOldParentTask->id(), pManager);
        correctSortPriorityOfChildren<IGroup>(pNewGroup, -1, iNewPosition, std::numeric_limits<int>::max(), 1, pManager);
      }
    }
    else if (nullptr != pOldGroup)
    {
      if (nullptr != pNewParentTask)
      {
        // task has been moved from a group to a parent task
        pTask->setGroup(pNewParentTask->group());
        pTask->setParentTask(pNewParentTask->id());
        fillSortPriorityGapsOfChildren<IGroup>(pOldGroup, -1, pManager);
        correctSortPriorityOfChildren<ITask>(pNewParentTask, pNewParentTask->id(), iNewPosition, std::numeric_limits<int>::max(), 1, pManager);
      }
      else if (pOldGroup == pNewGroup)
      {
        // order within the group has changed
        int iDelta = iNewPosition - iCurrentPosition;
        if (iDelta < 0)
        {
          correctSortPriorityOfChildren<IGroup>(pOldGroup, -1, iNewPosition, iCurrentPosition, 1, pManager);
        }
        else if (iDelta > 0)
        {
          correctSortPriorityOfChildren<IGroup>(pOldGroup, -1, iCurrentPosition, iNewPosition, -1, pManager);
        }
      }
      else if (nullptr != pNewGroup)
      {
        // task has been moved to a new group
        pTask->setGroup(pNewGroup->id());
        pTask->setParentTask(-1);
        fillSortPriorityGapsOfChildren<IGroup>(pOldGroup, -1, pManager);
        correctSortPriorityOfChildren<IGroup>(pNewGroup, -1, iNewPosition, std::numeric_limits<int>::max(), 1, pManager);
      }
    }


    auto prio = pTask->priority();
    prio.setPriority(0, iNewPosition);
    pTask->setPriority(prio);


    // update the widgets
    if (nullptr != pOldParentTaskWidget)
    {
      pOldParentTaskWidget->removeTask(pTaskWidget);
    }

    if (nullptr != pOldGroupWidget)
    {
      pOldGroupWidget->removeTask(pTaskWidget);
    }


    if (nullptr != pNewParentTaskWidget)
    {
      pNewParentTaskWidget->insertTask(pTaskWidget, iNewPosition);
    }
    else if (nullptr != pNewGroupWidget)
    {
      pNewGroupWidget->insertTask(pTaskWidget, iNewPosition);
    }
  }
}

MoveTaskCommand::MoveTaskCommand(task_id taskId,
                                 group_id oldGroupId,
                                 group_id newGroupId,
                                 task_id oldParentTaskId,
                                 task_id newParentTaskId,
                                 int iOldPosition,
                                 int iNewPosition,
                                 Manager* pManager,
                                 WidgetManager* pWidgetManager)
  : m_taskId(taskId),
    m_oldGroupId(oldGroupId),
    m_newGroupId(newGroupId),
    m_oldParentTaskId(oldParentTaskId),
    m_newParentTaskId(newParentTaskId),
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
           m_pManager->task(m_newParentTaskId), m_pWidgetManager->taskWidget(m_newParentTaskId),
           m_pManager->task(m_oldParentTaskId), m_pWidgetManager->taskWidget(m_oldParentTaskId),
           m_iOldPosition, m_pManager);
  }

void MoveTaskCommand::redo()
{
  moveTask(m_pManager->task(m_taskId), m_pWidgetManager->taskWidget(m_taskId),
           m_pManager->group(m_oldGroupId), m_pWidgetManager->groupWidget(m_oldGroupId),
           m_pManager->group(m_newGroupId), m_pWidgetManager->groupWidget(m_newGroupId),
           m_pManager->task(m_oldParentTaskId), m_pWidgetManager->taskWidget(m_oldParentTaskId),
           m_pManager->task(m_newParentTaskId), m_pWidgetManager->taskWidget(m_newParentTaskId),
           m_iNewPosition, m_pManager);
 }
