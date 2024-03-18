
#ifndef TASKCONTROLLERINTERFACE_H
#define TASKCONTROLLERINTERFACE_H

#include "id_types.h"

class QString;
class QUrl;

class ITaskController
{
public:
  virtual void renameTask(task_id, const QString&) = 0;
  virtual void changeTaskDescription(task_id, const QString&) = 0;
  virtual void startTimeTracking(task_id) = 0;
  virtual void stopTimeTracking(task_id) = 0;
  virtual void onPropertyChanged(task_id, const QString&, const QString&) = 0;
  virtual void onPropertyRemoved(task_id, const QString&) = 0;
  virtual void onTaskMoved(task_id, task_id, int) = 0;
  virtual void onTaskRemoved(task_id, task_id) = 0;
  virtual void onTaskDeleted(task_id) = 0;
  virtual void createNewSubTask(task_id) = 0;
  virtual void onAddTimeToTaskRequested(task_id) = 0;
  virtual void onRemoveTimeFromTaskRequested(task_id) = 0;
  virtual void onLinkAdded(task_id, QUrl) = 0;
  virtual void onLinkRemoved(task_id, QUrl) = 0;
  virtual void onLinkInserted(task_id, QUrl, int) = 0;
  virtual void onPriorityUpdateRequested(task_id) = 0;
  virtual void onTimeTrackingStopped(task_id) = 0;
  virtual void onChildPropertyChangeRequested(task_id, const QString&, const QString&, bool) = 0;
  virtual void onAutoPriorityUpdateRequested(task_id) = 0;

  protected:
  ITaskController() = default;
  virtual ~ITaskController() = default;
};

#endif // TASKCONTROLLERINTERFACE_H
