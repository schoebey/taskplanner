#ifndef TASK_H
#define TASK_H

#include "taskinterface.h"
#include "timefragment.h"
#include "priority.h"
#include "id_types.h"
#include "id_generator.h"
#include "serializableinterface.h"
#include "property.h"

#include <QString>

#include <vector>
#include <memory>
#include <set>

class Manager;

class Task;
typedef std::shared_ptr<Task> tspTask;

class Group;
typedef std::shared_ptr<Group> tspGroup;
typedef std::weak_ptr<Group> twpGroup;

class Task : public ITask, public ISerializable
{
public:
  using id_generator<task_id>::setId;

  Task(Manager* pManager, task_id id = -1);

  int version() const override;

  ESerializingError serialize(ISerializer* pSerializer) const override;

  EDeserializingError deserialize(ISerializer* pSerializer) override;

  QString name() const override;
  void setName(const QString& sName) override;

  QString description() const override;
  void setDescription(const QString& sDescription) override;

  SPriority priority() const override;
  void setPriority(const SPriority& priority) override;

  task_id parentTask() const override;
  void setParentTaskId(task_id parentTaskId);
  void setParentTask(task_id parentTaskId) override;

  std::set<task_id> taskIds() const override;
  bool addTask(task_id id) override;
  bool addTaskId(task_id id);
  bool removeTask(task_id id) override;

  void startWork() override;
  void stopWork() override;
  std::vector<STimeFragment> timeFragments() const override;
  void setTimeFragments(const std::vector<STimeFragment>& vFragments);

  group_id group() const override;
  void setGroup(group_id groupId) override;

  std::set<QString> propertyNames() const override;
  QString propertyValue(const QString& sName) const override;
  bool setPropertyValue(const QString& sName, const QString& sValue) override;

private:
  Manager* m_pManager = nullptr;
  SPriority m_priority;
  std::vector<STimeFragment> m_vTimingInfo;
  std::set<task_id> m_subTaskIds;
  group_id m_groupId = -1;
  task_id m_parentTaskId = -1;

  Properties m_properties;
};

#endif // TASK_H
