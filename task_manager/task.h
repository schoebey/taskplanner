#ifndef TASK_H
#define TASK_H

#include "timefragment.h"
#include "priority.h"

#include <QString>

#include <vector>
#include <memory>

class Task;
typedef std::shared_ptr<Task> tspTask;

class Task
{
public:
  Task();

  QString name() const;
  void setName(const QString& sName);

  QString description() const;
  void setDescription(const QString& sDescription);

  SPriority priority() const;
  void setPriority(const SPriority& priority);

  void startWork();
  void stopWork();

private:
  QString m_sName;
  QString m_sDescription;
  SPriority m_priority;
  std::vector<STimeFragment> m_vTimingInfo;
  std::vector<tspTask> m_vspSubTasks;
};

#endif // TASK_H
