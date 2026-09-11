#ifndef PLUGINEVENTBROKER_H
#define PLUGINEVENTBROKER_H

#include "id_types.h"
#include "libtaskmanager.h"

#include <QObject>
#include <QString>

// PluginEventBroker decouples plugin notification from MainWindow: it is the single
// QObject that plugins connect their slots to in order to observe application events
// (reminders firing, task subject/description edits), without MainWindow itself having
// to expose these as signals (MainWindow already derives from ITaskController and
// IGroupController, and Qt does not support inheriting from more than one QObject, so
// adding further signals directly on MainWindow via multiple QObject inheritance is not
// an option). MainWindow owns one instance and calls the notifyXxx() methods at the
// relevant points; the corresponding signals are emitted for any connected plugin.
class LIBTASKMANAGER PluginEventBroker : public QObject
{
  Q_OBJECT

public:
  explicit PluginEventBroker(QObject* pParent = nullptr);

  void notifyAlert(task_id taskId);
  void notifySubjectChanged(task_id taskId, const QString& sSubject);
  void notifyDescriptionChanged(task_id taskId, const QString& sDescription);

signals:
  void alert(task_id taskId);
  void subjectChanged(task_id taskId, const QString& sSubject);
  void descriptionChanged(task_id taskId, const QString& sDescription);
};

#endif // PLUGINEVENTBROKER_H
