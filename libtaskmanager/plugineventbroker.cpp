#include "plugineventbroker.h"

PluginEventBroker::PluginEventBroker(QObject* pParent) :
  QObject(pParent)
{

}

void PluginEventBroker::notifyAlert(task_id taskId)
{
  emit alert(taskId);
}

void PluginEventBroker::notifySubjectChanged(task_id taskId, const QString& sSubject)
{
  emit subjectChanged(taskId, sSubject);
}

void PluginEventBroker::notifyDescriptionChanged(task_id taskId, const QString& sDescription)
{
  emit descriptionChanged(taskId, sDescription);
}
