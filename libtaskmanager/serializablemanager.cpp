#include "serializablemanager.h"
#include "serializerinterface.h"
#include "task.h"
#include "group.h"
#include "tag.h"

SerializableManager::SerializableManager(Manager* pManager)
  : m_pManager(pManager)
{

}

int SerializableManager::version() const
{
  return 0;
}

void SerializableManager::clear()
{
  m_tasks.clear();
  m_groups.clear();
}

Task* SerializableManager::addTask(task_id taskId)
{
  tspTask spTask = std::make_shared<Task>(m_pManager, taskId);
  spTask->setProperty<QDateTime>("added date", QDateTime::currentDateTime());
  m_tasks[spTask->id()] = spTask;
  return spTask.get();
}

Task* SerializableManager::task(task_id id) const
{
  auto it = m_tasks.find(id);
  if (it != m_tasks.end())
  {
    return it->second.get();
  }

  return nullptr;
}

std::set<task_id> SerializableManager::taskIds() const
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
    // remove all children from the manager
    for (const auto& childId : it->second->taskIds())
    {
      removeTask(childId);
    }

    // remove the task from its parent if it has one
    auto pParent = task(it->second->parentTask());
    if (nullptr != pParent)
    {
      pParent->removeTask(id);
    }

    // remove the task from its group
    IGroup* pGroup = group(it->second->group());
    if (nullptr != pGroup)
    {
      pGroup->removeTask(id);
    }

    m_tasks.erase(it);
    return true;
  }

  return false;
}

bool SerializableManager::changeTaskId(task_id oldId, task_id newId)
{
  auto it = m_tasks.find(oldId);
  if (it != m_tasks.end())
  {
    tspTask spTask = it->second;
    m_tasks.erase(it);
    spTask->setId(newId);
    m_tasks[newId] = spTask;
  }

  return false;
}

Group* SerializableManager::addGroup(group_id groupId)
{
  tspGroup spGroup = std::make_shared<Group>(m_pManager, groupId);
  m_groups[spGroup->id()] = spGroup;
  return spGroup.get();
}

Group* SerializableManager::group(group_id id) const
{
  auto it = m_groups.find(id);
  if (it != m_groups.end())
  {
    return it->second.get();
  }

  return nullptr;
}

std::set<group_id> SerializableManager::groupIds() const
{
  std::set<group_id> groupIds;
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

bool SerializableManager::changeGroupId(group_id oldId, group_id newId)
{
  auto it = m_groups.find(oldId);
  if (it != m_groups.end())
  {
    tspGroup spGroup = it->second;
    m_groups.erase(it);
    spGroup->setId(newId);
    m_groups[newId] = spGroup;
  }

  return false;
}

Tag* SerializableManager::addTag(tag_id tagId)
{
  tspTag spTag = std::make_shared<Tag>(tagId);
  m_tags[spTag->id()] = spTag;
  return spTag.get();
}

Tag* SerializableManager::tag(tag_id id) const
{
  auto it = m_tags.find(id);
  if (it != m_tags.end())
  {
    return it->second.get();
  }

  return nullptr;
}

std::set<tag_id> SerializableManager::tagIds() const
{
  std::set<tag_id> tagIds;
  for (const auto& el : m_tags)
  {
    tagIds.insert(el.second->id());
  }
  return tagIds;
}

bool SerializableManager::modifyTag(tag_id tagId, const QString& sNewName, const QColor& col)
{
  auto pTag = tag(tagId);
  if (nullptr != pTag)
  {
    pTag->setName(sNewName);
    pTag->setColor(col);
    return true;
  }
  return false;
}

bool SerializableManager::removeTag(tag_id tagId)
{
  auto it = m_tags.find(tagId);
  if (it != m_tags.end())
  {
    m_tags.erase(it);
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

  err= pSerializer->deinitSerialization();
  if (ESerializingError::eOk != err)  { return err; }

  return ESerializingError::eOk;
}

void SerializableManager::rebuildHierarchy()
{
  for (const auto& groupId : groupIds())
  {
    auto pGroup = group(groupId);
    if (nullptr != pGroup)
    {
      for (const auto& taskId : pGroup->taskIds())
      {
        auto pTask = task(taskId);
        if (nullptr != pTask)
        {
          pTask->setGroup(pGroup->id());
        }
      }
    }
  }
}

EDeserializingError SerializableManager::deserialize(ISerializer* pSerializer)
{
  clear();

  EDeserializingError err;
  err= pSerializer->initDeserialization();
  if (EDeserializingError::eOk != err)  { return err; }

  err= pSerializer->deserialize(*this);
  if (EDeserializingError::eOk != err)  { return err; }

  // add tasks to groups
  rebuildHierarchy();

  err= pSerializer->deinitDeserialization();
  if (EDeserializingError::eOk != err)  { return err; }

  return EDeserializingError::eOk;
}
