#ifndef GROUP_H
#define GROUP_H

#include "groupinterface.h"
#include "id_types.h"
#include "serializableinterface.h"

#include <QString>

#include <set>
#include <memory>

class Manager;

class Task;
typedef std::shared_ptr<Task> tspTask;

class Group: public IGroup, public ISerializable
{
public:
  using id_generator<group_id>::setId;

  Group(Manager* pManager, group_id id = -1);

  int version() const override;

  ESerializingError serialize(ISerializer* pSerializer) const override;

  EDeserializingError deserialize(ISerializer* pSerializer) override;

  QString name() const override;
  void setName(const QString& sName) override;

  QString description() const override;
  void setDescription(const QString& sDescription) override;

  std::set<task_id> taskIds() const override;
  bool addTask(task_id id) override;
  bool removeTask(task_id id) override;

  bool addTaskId(task_id taskId);

private:
  Manager* m_pManager = nullptr;
  QString m_sName;
  QString m_sDescription;
  std::set<task_id> m_tasksIds;
};

#endif // GROUP_H
