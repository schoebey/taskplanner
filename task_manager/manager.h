#ifndef MANAGER_H
#define MANAGER_H

#include "id_types.h"
#include "serializableinterface.h"

#include <memory>
#include <set>
#include <map>

class ITask;
class Task;
typedef std::shared_ptr<Task> tspTask;

class IGroup;
class Group;
typedef std::shared_ptr<Group> tspGroup;


class SerializableManager;
class Manager
{
public:
  Manager();

  ESerializingError serializeTo(ISerializer* pSerializer) const;

  EDeserializingError deserializeFrom(ISerializer* pSerializer);

  ITask* addTask(task_id taskId = -1);
  ITask* task(task_id id) const;
  std::set<task_id> taskIds() const;
  bool removeTask(task_id);

  IGroup* addGroup(group_id groupId = -1);
  IGroup* group(group_id id) const;
  std::set<group_id> groupIds() const;
  bool removeGroup(group_id id);


private:
  std::shared_ptr<SerializableManager> m_spPrivate;
};

#endif // MANAGER_H
