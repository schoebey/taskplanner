#include "task.h"
#include "group.h"
#include "manager.h"
#include "serializerinterface.h"

Task::Task(Manager* pManager, task_id id)
 : ITask(id),
   m_pManager(pManager)
{

}

int Task::version() const
{
  return 0;
}

ESerializingError Task::serialize(ISerializer* pSerializer) const
{
  return pSerializer->serialize(*this);
}

EDeserializingError Task::deserialize(ISerializer* pSerializer)
{
  return pSerializer->deserialize(*this);
}

QString Task::name() const
{
  return m_sName;
}

void Task::setName(const QString& sName)
{
  m_sName = sName;
}

QString Task::description() const
{
  return m_sDescription;
}

void Task::setDescription(const QString& sDescription)
{
  m_sDescription = sDescription;
}

SPriority Task::priority() const
{
  return m_priority;
}

void Task::setPriority(const SPriority& priority)
{
  m_priority = priority;
}

task_id Task::parentTask() const
{
  return m_parentTaskId;
}

void Task::setParentTaskId(task_id parentTaskId)
{
  m_parentTaskId = parentTaskId;
}

void Task::setParentTask(task_id parentTaskId)
{
  if (m_parentTaskId != parentTaskId)
  {
    ITask* pParent = m_pManager->task(m_parentTaskId);
    if (nullptr != pParent)
    {
      pParent->removeTask(id());
    }

    setParentTaskId(parentTaskId);
    pParent = m_pManager->task(parentTaskId);
    if (nullptr != pParent)
    {
      pParent->addTask(id());
    }
  }
}

std::set<task_id> Task::taskIds() const
{
  return m_subTaskIds;
}

bool Task::addTask(task_id taskId)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask && addTaskId(taskId))
  {
    pTask->setParentTask(id());
    return true;
  }

  return false;
}

bool Task::addTaskId(task_id id)
{
  if (id == this->id())  { return false; }

  return m_subTaskIds.insert(id).second;
}

bool Task::removeTask(task_id id)
{
  auto it = m_subTaskIds.find(id);
  if (it != m_subTaskIds.end())
  {
    m_subTaskIds.erase(it);

    ITask* pTask = m_pManager->task(id);
    if (nullptr != pTask)
    {
      pTask->setParentTask(-1);
    }
    return true;
  }

  return false;
}

void Task::startWork()
{
  STimeFragment fragment;
  fragment.startTime = QDateTime::currentDateTime();
  m_vTimingInfo.push_back(fragment);
}

void Task::stopWork()
{
  if (m_vTimingInfo.empty())  { return; }

  if (!m_vTimingInfo.back().stopTime.isValid())
  {
    m_vTimingInfo.back().stopTime = QDateTime::currentDateTime();
  }
}

std::vector<STimeFragment> Task::timeFragments() const
{
  return m_vTimingInfo;
}

void Task::setTimeFragments(const std::vector<STimeFragment>& vFragments)
{
  m_vTimingInfo = vFragments;
}

group_id Task::group() const
{
  return m_groupId;
}

void Task::setGroup(group_id groupId)
{
  if (m_groupId != groupId)
  {
    IGroup* pGroup = m_pManager->group(m_groupId);
    if (nullptr != pGroup)
    {
      pGroup->removeTask(id());
    }

    m_groupId = groupId;

    pGroup = m_pManager->group(m_groupId);
    if (nullptr != pGroup)
    {
      pGroup->addTask(id());
    }
  }

  for (const auto& subTaskId : m_subTaskIds)
  {
    ITask* pSubTask = m_pManager->task(subTaskId);
    if (nullptr != pSubTask)  { pSubTask->setGroup(groupId); }
  }
}
