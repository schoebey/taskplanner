#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include "id_types.h"
#include "propertyproviderinterface.h"

#include <QFrame>

namespace Ui {
  class GroupWidget;
}

class TaskWidget;
class GroupWidget : public QFrame, public IPropertyProvider
{
  Q_OBJECT

public:
  explicit GroupWidget(group_id id, QWidget *parent = nullptr);
  ~GroupWidget() override;

  group_id id() const;

  void setName(const QString& sName);

  bool setPropertyValue(const QString& sName, const QString& sValue) override;
  bool removeProperty(const QString& sName) override;
  std::set<QString> propertyNames() const override;
  bool hasPropertyValue(const QString& sName) const override;
  QString propertyValue(const QString& sName) const override;

  void requestInsert(TaskWidget* pTaskWidget, int iPos = -1);
  void insertTask(TaskWidget* pTaskWidget, int iPos = -1);
  void removeTask(TaskWidget* pTaskWidget);

  static GroupWidget* GroupWidgetUnderMouse();

  int indexFromPoint(QPoint pt);

  Q_PROPERTY(QImage backgroundImage READ backgroundImage WRITE setBackgroundImage)
  QImage backgroundImage() const;
  void setBackgroundImage(const QImage& img);

  void reorderTasks(const std::vector<task_id>& vIds);

  void setAutoSortingEnabled(bool bEnabled);

signals:
  void taskMovedTo(task_id taskId, group_id groupId, int iPos);
  void renamed(group_id groupId, const QString& sNewName);
  void newTaskClicked(group_id);
  void autoSortEnabled(group_id);
  void autoSortDisabled(group_id);

protected:
  void resizeEvent(QResizeEvent* pEvent);
  void moveEvent(QMoveEvent* pEvent);
  bool eventFilter(QObject* pObj, QEvent* pEvent);
  void ShowGhost(TaskWidget* pTaskWidget, int iPos);
  void UpdatePositions(int iSpace = -1, int iSpacePos = 0);
  TaskWidget* taskWidgetAt(QPoint pt);
  void setUpContextMenu();

protected slots:
  void repositionChildren();
  void onNewTaskClicked();
  void onTitleEdited();
  void onSortClicked(bool bChecked);

private:
  Ui::GroupWidget *ui;
  group_id m_groupId;
  std::vector<TaskWidget*> m_vpTaskWidgets;
  QImage m_backgroundImage;

  static GroupWidget* m_pMouseHoveringOver;
};

#endif // GROUPWIDGET_H
