#include "group.h"
#include "task.h"
#include "manager.h"
#include "serializerinterface.h"

Group::Group(Manager* pManager, group_id id)
  : IGroup(id),
    m_pManager(pManager)
{

}

int Group::version() const
{
  return 0;
}

ESerializingError Group::serialize(ISerializer* pSerializer) const
{
  return pSerializer->serialize(*this);
}

EDeserializingError Group::deserialize(ISerializer* pSerializer)
{
  return pSerializer->deserialize(*this);
}

QString Group::name() const
{
  return m_sName;
}

void Group::setName(const QString& sName)
{
  m_sName = sName;
}

QString Group::description() const
{
  return m_sDescription;
}

void Group::setDescription(const QString& sDescription)
{
  m_sDescription = sDescription;
}

std::set<task_id> Group::taskIds() const
{
  return m_tasksIds;
}

bool Group::addTaskId(task_id taskId)
{
  return m_tasksIds.insert(taskId).second;
}

bool Group::addTask(task_id taskId)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask && addTaskId(taskId))
  {
    pTask->setGroup(id());
    return true;
  }

  return false;
}

bool Group::removeTask(task_id taskId)
{
  auto it = std::find(m_tasksIds.begin(), m_tasksIds.end(), taskId);
  if (it != m_tasksIds.end())
  {
    m_tasksIds.erase(it);

    ITask* pTask = m_pManager->task(taskId);
    if (nullptr != pTask)
    {
      pTask->setGroup(-1);
    }

    return true;
  }

  return false;
}
