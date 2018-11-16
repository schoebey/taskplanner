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

void Task::setParentTask(task_id parentTaskId)
{
  m_parentTaskId = parentTaskId;
}

std::set<task_id> Task::taskIds() const
{
  return m_subTaskIds;
}

bool Task::addTask(task_id id)
{
  return m_subTaskIds.insert(id).second;
}

bool Task::removeTask(task_id id)
{
  auto it = m_subTaskIds.find(id);
  if (it != m_subTaskIds.end())
  {
    m_subTaskIds.erase(it);
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

  m_vTimingInfo.back().stopTime = QDateTime::currentDateTime();
}

void Task::setGroup(group_id groupId)
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

  for (const auto& subTaskId : m_subTaskIds)
  {
    ITask* pSubTask = m_pManager->task(subTaskId);
    if (nullptr != pSubTask)  { pSubTask->setGroup(groupId); }
  }
}
