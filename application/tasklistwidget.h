#ifndef TASKLISTWIDGET_H
#define TASKLISTWIDGET_H

#include "itaskcontainerwidget.h"

#include <QFrame>
#include <QPoint>


#include "id_types.h"

class TaskWidget;
class QResizeEvent;
class QMoveEvent;

class TaskListWidget : public QFrame, public ITaskContainerWidget
{
  Q_OBJECT

public:
  TaskListWidget(QWidget* pParent = nullptr);

  static TaskListWidget* TaskListWidgetUnderMouse();

  void requestInsert(TaskWidget* pTaskWidget, int iPos = -1) override;
  bool insertTask(TaskWidget* pTaskWidget, int iPos = -1) override;
  void removeTask(TaskWidget* pTaskWidget) override;

  void setBackgroundImage(const QImage& img);
  QImage backgroundImage();

  void reorderTasks(const std::vector<task_id> &vIds);

  int indexFromPoint(QPoint pt);

  bool onMouseMoved(const QPoint &pt);

  void setAutoResize(bool bAutoResize);

  QSize sizeHint() const override;
signals:
  void taskInserted(TaskWidget* pTaskWidget, int iPos);
  void taskRemoved(TaskWidget* pTaskWidget);
  void sizeChanged();

private:
  void ShowGhost(TaskWidget* pTaskWidget, int iPos);
  TaskWidget* taskWidgetAt(QPoint pt);

  void resizeEvent(QResizeEvent* pEvent) override;
  void moveEvent(QMoveEvent* pEvent) override;
  QSize minimumSizeHint() const override;

private slots:
  void updateTaskPositions();
  void updatePositions(int iSpace = -1, int iSpacePos = 0);
  void setSize(int iWidth, int iHeight);

private:
  std::vector<TaskWidget*> m_vpTaskWidgets;
  static TaskListWidget* m_pMouseHoveringOver;
  QImage m_backgroundImage;
  bool m_bAutoResize = false;
  QSize m_minimumSize;
};

#endif // TASKLISTWIDGET_H
