#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include "id_types.h"
#include "propertyproviderinterface.h"
#include "itaskcontainerwidget.h"

#include <QFrame>

namespace Ui {
  class GroupWidget;
}

class TaskWidget;
class GroupWidget : public QFrame, public IPropertyProvider, public ITaskContainerWidget
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


  int indexFromPoint(QPoint pt);

  Q_PROPERTY(QImage backgroundImage READ backgroundImage WRITE setBackgroundImage)
  QImage backgroundImage() const;
  void setBackgroundImage(const QImage& img);

  void reorderTasks(const std::vector<task_id>& vIds);

  void setAutoSortingEnabled(bool bEnabled);

  void requestInsert(TaskWidget* pTaskWidget, int iPos = -1) override;
  bool insertTask(TaskWidget* pTaskWidget, int iPos = -1) override;
  void removeTask(TaskWidget* pTaskWidget) override;
  std::vector<TaskWidget*> tasks() const override;
  void ensureVisible(TaskWidget* pTaskWidget) override;

  bool onMouseMoved(const QPoint &pt);

signals:
  void taskMovedTo(task_id taskId, group_id groupId, int iPos);
  void renamed(group_id groupId, const QString& sNewName);
  void newTaskClicked(group_id);
  void autoSortEnabled(group_id);
  void autoSortDisabled(group_id);

protected:
  void ShowGhost(TaskWidget* pTaskWidget, int iPos);
  TaskWidget* taskWidgetAt(QPoint pt);
  void setUpContextMenu();

protected slots:
  void onTaskInserted(TaskWidget* pTaskWidget, int iPos = -1);
  void onNewTaskClicked();
  void onTitleEdited();
  void onSortClicked(bool bChecked);

private:
  Q_DISABLE_COPY(GroupWidget)
  
  Ui::GroupWidget *ui;
  group_id m_groupId;
  QImage m_backgroundImage;
};

#endif // GROUPWIDGET_H
