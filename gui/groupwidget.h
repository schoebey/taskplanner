#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include <QFrame>
#include "id_types.h"

namespace Ui {
  class GroupWidget;
}

class TaskWidget;
class GroupWidget : public QFrame
{
  Q_OBJECT

public:
  explicit GroupWidget(group_id id, QWidget *parent = nullptr);
  ~GroupWidget();

  group_id id() const;

  void setName(const QString& sName);

  void setCanvas(QWidget* pCanvas);

  void InsertTask(TaskWidget* pTaskWidget, int iPos = -1);
  void RemoveTask(TaskWidget* pTaskWidget);

  static GroupWidget* GroupWidgetUnderMouse();

  int indexFromPoint(QPoint pt);

signals:
  void taskMovedTo(task_id taskId, group_id groupId, int iPos);
  void renamed(group_id groupId, const QString& sNewName);
  void newTaskClicked(group_id);

protected:
  void resizeEvent(QResizeEvent* pEvent);
  void moveEvent(QMoveEvent* pEvent);
  void ShowGhost(TaskWidget* pTaskWidget, int iPos);
  void repositionChildren();

protected slots:
  void onNewTaskClicked();
  void onTitleEdited();

private:
  Ui::GroupWidget *ui;
  group_id m_groupId;
  std::vector<TaskWidget*> m_vpTaskWidgets;
  bool eventFilter(QObject* pObj, QEvent* pEvent);

  QWidget* m_pCanvas = nullptr;
  static GroupWidget* m_pMouseHoveringOver;
};

#endif // GROUPWIDGET_H
