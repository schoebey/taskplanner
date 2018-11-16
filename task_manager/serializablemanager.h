#ifndef SERIALIZABLEMANAGER_H
#define SERIALIZABLEMANAGER_H

#include "manager.h"
#include "serializableinterface.h"

class SerializableManager : public ISerializable
{
public:
  SerializableManager(Manager* pManager);

  int version() const override;

  ESerializingError serialize(ISerializer* pSerializer) const override;

  EDeserializingError deserialize(ISerializer* pSerializer) override;

  Task* addTask(task_id taskId = -1);
  Task* task(task_id id);
  std::set<task_id> taskIds();
  bool removeTask(task_id);

  Group* addGroup(group_id groupId = -1);
  Group* group(group_id id);
  std::set<group_id> groupIds();
  bool removeGroup(group_id id);

private:
  std::map<task_id, tspTask> m_tasks;
  std::map<group_id, tspGroup> m_groups;
  Manager* m_pManager = nullptr;
};

#endif // SERIALIZABLEMANAGER_H
