#ifndef TASKINTERFACE_H
#define TASKINTERFACE_H

#include "priority.h"
#include "timefragment.h"
#include "id_types.h"
#include "id_generator.h"
#include "propertyproviderinterface.h"

#include <QString>

#include <set>

class ITask : public id_generator<task_id>, public IPropertyProvider
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

  virtual void startWork(const QDateTime& when = QDateTime::currentDateTime()) = 0;
  virtual void stopWork(const QDateTime& when = QDateTime::currentDateTime()) = 0;

  /*!
   * \brief insetTimeFragment inserts a fragment of time into the collection
   * of already tracked time
   * \param start start date and time of the fragment
   * \param end end date and time of the fragment
   */
  virtual void insertTimeFragment(const QDateTime& start, const QDateTime& end) = 0;

  /*!
   * \brief removeTimeFragment checks all available time fragments and alters them
   * to make sure they don't overlap with the range \a start and \a end.
   * This means either the start or end of a fragment can be moved, or
   * the fragment could be deleted entirely.
   * \param start start date and time of the fragment
   * \param end end date and time of the fragment
   */
  virtual void removeTimeFragment(const QDateTime& start, const QDateTime& end) = 0;
  virtual bool isTrackingTime() const = 0;
  virtual std::vector<STimeFragment> timeFragments() const = 0;

  virtual std::vector<QString> tags() const = 0;
  virtual bool setTags(const std::vector<QString>& vsTagNames) = 0;

  virtual group_id group() const = 0;
  virtual void setGroup(group_id groupId) = 0;

protected:
  ITask(task_id id = -1) : id_generator<task_id>(id) {}
  ~ITask() {}
};

#endif // TASKINTERFACE_H
