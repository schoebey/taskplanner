#include "manager.h"
#include "tag.h"
#include "task.h"
#include "group.h"
#include "serializerinterface.h"
#include "serializablemanager.h"

Manager::Manager()
{
  m_spPrivate = std::make_shared<SerializableManager>(this);
}

Manager::~Manager()
{

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

ITask* Manager::task(task_id id) const
{
  return m_spPrivate->task(id);
}

std::set<task_id> Manager::taskIds() const
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

IGroup* Manager::group(group_id id) const
{
  return m_spPrivate->group(id);
}

std::set<group_id> Manager::groupIds() const
{
  return m_spPrivate->groupIds();
}

bool Manager::removeGroup(group_id id)
{
  return m_spPrivate->removeGroup(id);
}

ITag* Manager::addTag(tag_id tagId)
{
  return m_spPrivate->addTag(tagId);
}

ITag* Manager::tag(tag_id id) const
{
  return m_spPrivate->tag(id);
}

std::set<tag_id> Manager::tagIds() const
{
  return m_spPrivate->tagIds();
}

bool Manager::modifyTag(tag_id tagId, const QString& sNewName, const QColor& col)
{
  return m_spPrivate->modifyTag(tagId, sNewName, col);
}

bool Manager::removeTag(tag_id tagId)
{
  return m_spPrivate->removeTag(tagId);
}
