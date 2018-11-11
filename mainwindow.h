#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "id_types.h"
#include <QMainWindow>

namespace Ui {
  class MainWindow;
}

class GroupWidget;
class TaskWidget;
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();


  GroupWidget* createGroupWidget(group_id id);
  TaskWidget* createTaskWidget(task_id id);

private slots:
  void createNewTask(group_id groupId);
  void renameGroup(group_id id, const QString& sNewName);
  void renameTask(task_id id, const QString& sNewName);
  void changeTaskDescription(task_id id, const QString& sNewDescr);
  void moveTask(task_id id, group_id groupId, int iPos);

private:
  Ui::MainWindow *ui;
  std::map<group_id, GroupWidget*> m_groupWidgets;
  std::map<task_id, TaskWidget*> m_taskWidgets;
};

#endif // MAINWINDOW_H
