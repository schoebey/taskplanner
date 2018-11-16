#include "manager.h"
#include "task.h"
#include "group.h"
#include "serializerinterface.h"
#include "serializablemanager.h"

Manager::Manager()
{
  m_spPrivate = std::make_shared<SerializableManager>(this);
}

ESerializingError Manager::serializeTo(ISerializer* pSerializer) const
{
  return m_spPrivate->serialize(pSerializer);
}

EDeserializingError Manager::deserializeFrom(ISerializer* pSerializer)
{
  return m_spPrivate->deserialize(pSerializer);
}

ITask* Manager::addTask(task_id taskId)
{
  return m_spPrivate->addTask(taskId);
}

ITask* Manager::task(task_id id)
{
  return m_spPrivate->task(id);
}

std::set<task_id> Manager::taskIds()
{
  return m_spPrivate->taskIds();
}

bool Manager::removeTask(task_id id)
{
  return m_spPrivate->removeTask(id);
}

IGroup* Manager::addGroup(group_id groupId)
{
  return m_spPrivate->addGroup(groupId);
}

IGroup* Manager::group(group_id id)
{
  return m_spPrivate->group(id);
}

std::set<group_id> Manager::groupIds()
{
  return m_spPrivate->groupIds();
}

bool Manager::removeGroup(group_id id)
{
  return m_spPrivate->removeGroup(id);
}

