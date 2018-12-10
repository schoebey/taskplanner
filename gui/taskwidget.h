#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include "id_types.h"

#include <QFrame>

#include <map>

namespace Ui {
  class TaskWidget;
}

class GroupWidget;
class EditableLabel;
class TaskWidget : public QFrame
{
  Q_OBJECT

public:
  explicit TaskWidget(task_id id, QWidget *parent = nullptr);
  ~TaskWidget() override;

  task_id id() const;

  void setName(const QString& sName);
  void setDescription(const QString& sDescription);

  void SetGroupWidget(GroupWidget* pGroupWidget);

  static TaskWidget* DraggingTaskWidget();

  void addProperty(const QString& sName, const QString& sValue);
  void setPropertyValue(const QString& sName, const QString& sValue);

public slots:
  void setExpanded(bool bExpanded);

signals:
  void renamed(task_id taskId, const QString& sNewName);
  void descriptionChanged(task_id taskId, const QString& sNewDescription);
  void timeTrackingStarted(task_id id);
  void timeTrackingStopped(task_id id);
  void propertyChanged(task_id id, const QString& sName, const QString& sValue);
  void sizeChanged();

private slots:
  void onTitleEdited();
  void onDescriptionEdited();
  void on_pStartStop_toggled(bool bOn);
  void onTimeTrackingStopped(task_id id);
  void onPropertyEdited();
  void onAddPropertyTriggered();
  void updateSize();

private:
  bool eventFilter(QObject* pObj, QEvent* pEvent) override;

  void setUpContextMenu();

private:
  Ui::TaskWidget *ui;
  task_id m_taskId;
  void mousePressEvent(QMouseEvent* pMouseEvent) override;
  void mouseMoveEvent(QMouseEvent* pMouseEvent) override;
  void paintEvent(QPaintEvent* pEvent) override;

  bool m_bMouseDown = false;
  QPoint m_mouseDownPos;
  QImage m_backgroundImage;

  GroupWidget* m_pGroupWidget = nullptr;
  GroupWidget* m_pPreviousGroupWidget = nullptr;

  std::map<QString, EditableLabel*> m_propertyLineEdits;

  static TaskWidget* m_pDraggingTaskWidget;
};

#endif // TASKWIDGET_H
