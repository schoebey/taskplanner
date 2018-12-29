#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "id_types.h"
#include <QMainWindow>
#include <map>

namespace Ui {
  class MainWindow;
}

class GroupWidget;
class TaskWidget;
class Manager;
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(Manager* pManager, QWidget *parent = nullptr);
  ~MainWindow();


  GroupWidget* createGroupWidget(group_id id);
  TaskWidget* createTaskWidget(task_id id);

private slots:
  void createNewTask(group_id groupId);
  void renameGroup(group_id id, const QString& sNewName);
  void renameTask(task_id id, const QString& sNewName);
  void changeTaskDescription(task_id id, const QString& sNewDescr);
  void moveTask(task_id id, group_id groupId, int iPos);
  void reloadStylesheet(const QString& sPath);
  void load();
  void startTimeTracking(task_id taskId);
  void stopTimeTracking(task_id taskId);
  void on_actionOpen_triggered();
  void on_actionSaveAs_triggered();
  void on_actionReport_triggered();
  void on_actionDisplayReport_triggered();
  void onPropertyChanged(task_id taskId, const QString& sPropertyName, const QString& sValue);
  void onTaskRemoved(task_id parentTaskId, task_id childTaskId);
  void onTaskAdded(task_id parentTaskId, task_id childTaskId);

signals:
  void timeTrackingStopped(task_id taskId);

private:
  Ui::MainWindow *ui;
  std::map<group_id, GroupWidget*> m_groupWidgets;
  std::map<task_id, TaskWidget*> m_taskWidgets;
  Manager* m_pManager = nullptr;
};

#endif // MAINWINDOW_H
