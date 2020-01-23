#ifndef GROUPINTERFACE_H
#define GROUPINTERFACE_H

#include "id_types.h"
#include "id_generator.h"
#include "propertyproviderinterface.h"

#include <QString>

#include <set>

class IGroup : public id_generator<group_id>, public IPropertyProvider
{
public:
  virtual QString name() const = 0;
  virtual void setName(const QString& sName) = 0;

  virtual QString description() const = 0;
  virtual void setDescription(const QString& sDescription) = 0;

  virtual std::set<task_id> taskIds() const = 0;
  virtual bool addTask(task_id id) = 0;
  virtual bool removeTask(task_id id) = 0;

protected:
  IGroup(group_id id = -1) : id_generator<group_id>(id) {}
  ~IGroup() {}
};

#endif // GROUPINTERFACE_H
