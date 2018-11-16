#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <QFrame>
#include "id_types.h"

namespace Ui {
  class TaskWidget;
}

class GroupWidget;
class TaskWidget : public QFrame
{
  Q_OBJECT

public:
  explicit TaskWidget(task_id id, QWidget *parent = nullptr);
  ~TaskWidget();

  task_id id() const;

  void setName(const QString& sName);
  void setDescription(const QString& sDescription);

  void SetGroupWidget(GroupWidget* pGroupWidget);

  static TaskWidget* DraggingTaskWidget();

signals:
  void renamed(task_id taskId, const QString& sNewName);
  void descriptionChanged(task_id taskId, const QString& sNewDescription);
  void timeTrackingStarted(task_id id);
  void timeTrackingStopped(task_id id);

private slots:
  void onTitleEdited();
  void onDescriptionEdited();
  void on_pStartStop_toggled(bool bOn);

private:
  bool eventFilter(QObject* pObj, QEvent* pEvent);

private:
  Ui::TaskWidget *ui;
  task_id m_taskId;
  void mousePressEvent(QMouseEvent* pMouseEvent);
  void mouseMoveEvent(QMouseEvent* pMouseEvent);

  bool m_bMouseDown = false;
  QPoint m_mouseDownPos;

  GroupWidget* m_pGroupWidget = nullptr;
  GroupWidget* m_pPreviousGroupWidget = nullptr;

  static TaskWidget* m_pDraggingTaskWidget;
};

#endif // TASKWIDGET_H
