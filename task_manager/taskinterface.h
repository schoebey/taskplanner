#ifndef TASKINTERFACE_H
#define TASKINTERFACE_H

#include "priority.h"
#include "timefragment.h"
#include "id_types.h"
#include "id_generator.h"

#include <QString>

#include <set>

class ITask : public id_generator<task_id>
{
public:
  virtual QString name() const = 0;
  virtual void setName(const QString& sName) = 0;

  virtual QString description() const = 0;
  virtual void setDescription(const QString& sDescription) = 0;

  virtual SPriority priority() const = 0;
  virtual void setPriority(const SPriority& priority) = 0;
  virtual double autoPriority() const = 0;

  virtual task_id parentTask() const = 0;
  virtual void setParentTask(task_id parentTaskId) = 0;

  virtual std::set<task_id> taskIds() const = 0;
  virtual bool addTask(task_id id) = 0;
  virtual bool removeTask(task_id id) = 0;

  virtual void startWork() = 0;
  virtual void stopWork() = 0;
  virtual std::vector<STimeFragment> timeFragments() const = 0;

  virtual group_id group() const = 0;
  virtual void setGroup(group_id groupId) = 0;

  virtual std::set<QString> propertyNames() const = 0;
  virtual bool hasPropertyValue(const QString& sName) const = 0;
  virtual QString propertyValue(const QString& sName) const = 0;
  virtual bool setPropertyValue(const QString& sName, const QString& sValue) = 0;


protected:
  ITask(task_id id = -1) : id_generator<task_id>(id) {}
  ~ITask() {}
};

#endif // TASKINTERFACE_H
