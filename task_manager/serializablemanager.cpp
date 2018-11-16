#include "serializablemanager.h"
#include "serializerinterface.h"
#include "task.h"
#include "group.h"

SerializableManager::SerializableManager(Manager* pManager)
  : m_pManager(pManager)
{

}

int SerializableManager::version() const
{
  return 0;
}

Task* SerializableManager::addTask(task_id taskId)
{
  tspTask spTask = std::make_shared<Task>(m_pManager, taskId);
  m_tasks[spTask->id()] = spTask;
  return spTask.get();
}

Task* SerializableManager::task(task_id id)
{
  auto it = m_tasks.find(id);
  if (it != m_tasks.end())
  {
    return it->second.get();
  }

  return nullptr;
}

std::set<task_id> SerializableManager::taskIds()
{
  std::set<task_id> taskIds;
  for (const auto& el : m_tasks)
  {
    taskIds.insert(el.second->id());
  }
  return taskIds;
}

bool SerializableManager::removeTask(task_id id)
{
  auto it = m_tasks.find(id);
  if (it != m_tasks.end())
  {
    m_tasks.erase(it);
    return true;
  }

  return false;
}

Group* SerializableManager::addGroup(group_id groupId)
{
  tspGroup spGroup = std::make_shared<Group>(m_pManager, groupId);
  m_groups[spGroup->id()] = spGroup;
  return spGroup.get();
}

Group* SerializableManager::group(group_id id)
{
  auto it = m_groups.find(id);
  if (it != m_groups.end())
  {
    return it->second.get();
  }

  return nullptr;
}

std::set<group_id> SerializableManager::groupIds()
{
  std::set<task_id> groupIds;
  for (const auto& el : m_groups)
  {
    groupIds.insert(el.second->id());
  }
  return groupIds;
}

bool SerializableManager::removeGroup(group_id id)
{
  auto it = m_groups.find(id);
  if (it != m_groups.end())
  {
    m_groups.erase(it);
    return true;
  }

  return false;
}

ESerializingError SerializableManager::serialize(ISerializer* pSerializer) const
{
  ESerializingError err;
  err = pSerializer->initSerialization();
  if (ESerializingError::eOk != err)  { return err; }

  // write out information about contained groups and tasks (e.g. how many there are)
  err = pSerializer->serialize(*this);
  if (ESerializingError::eOk != err)  { return err; }


  // write all the groups to the stream
  for (const auto& group : m_groups)
  {
    err = group.second->serialize(pSerializer);
    if (ESerializingError::eOk != err)  { return err; }
  }

  // write all the tasks to the stream
  for (const auto& task : m_tasks)
  {
    err = task.second->serialize(pSerializer);
    if (ESerializingError::eOk != err)  { return err; }
  }

  err = pSerializer->deinitSerialization();
  if (ESerializingError::eOk != err)  { return err; }

  return ESerializingError::eOk;
}

EDeserializingError SerializableManager::deserialize(ISerializer* pSerializer)
{
  EDeserializingError err;
  err= pSerializer->initDeserialization();
  if (EDeserializingError::eOk != err)  { return err; }

  err= pSerializer->deserialize(*this);
  if (EDeserializingError::eOk != err)  { return err; }

  err= pSerializer->deinitDeserialization();
  if (EDeserializingError::eOk != err)  { return err; }

  return EDeserializingError::eOk;
}
