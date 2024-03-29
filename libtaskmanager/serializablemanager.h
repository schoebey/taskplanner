#ifndef SERIALIZABLEMANAGER_H
#define SERIALIZABLEMANAGER_H

#include "libtaskmanager.h"
#include "manager.h"
#include "serializableinterface.h"

class LIBTASKMANAGER SerializableManager : public ISerializable
{
public:
  SerializableManager(Manager* pManager);

  int version() const override;

  ESerializingError serialize(ISerializer* pSerializer) const override;

  EDeserializingError deserialize(ISerializer* pSerializer) override;

  Task* addTask(task_id taskId = -1);
  Task* task(task_id id) const;
  std::set<task_id> taskIds() const;
  bool removeTask(task_id);
  bool changeTaskId(task_id oldId, task_id newId);

  Group* addGroup(group_id groupId = -1);
  Group* group(group_id id) const;
  std::set<group_id> groupIds() const;
  bool removeGroup(group_id id);
  bool changeGroupId(group_id oldId, group_id newId);

  void clear();
private:
  std::map<task_id, tspTask> m_tasks;
  std::map<group_id, tspGroup> m_groups;
  Manager* m_pManager = nullptr;
  void rebuildHierarchy();
};

#endif // SERIALIZABLEMANAGER_H
