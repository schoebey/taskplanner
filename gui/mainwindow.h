#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "id_types.h"
#include <QMainWindow>
#include <QPointer>
#include <map>

namespace Ui {
  class MainWindow;
}

class GroupWidget;
class TaskWidget;
class Manager;
class TaskCreationDialog;
class QSignalMapper;
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(Manager* pManager, QWidget *parent = nullptr);
  ~MainWindow();


  GroupWidget* createGroupWidget(group_id id);
  TaskWidget* createTaskWidget(task_id id);

  bool loadFile(const QString& sFileName, QString* psErrorMessage = nullptr);
  
  bool saveFile(const QString& sFileName, QString* psErrorMessage = nullptr);
  
private slots:
  void createNewTask(group_id groupId);
  void renameGroup(group_id id, const QString& sNewName);
  void renameTask(task_id id, const QString& sNewName);
  void changeTaskDescription(task_id id, const QString& sNewDescr);
  void moveTask(task_id id, group_id groupId, int iPos);
  void reloadStylesheet(const QString& sPath);
  void initUi();
  void startTimeTracking(task_id taskId);
  void stopTimeTracking(task_id taskId);
  void on_actionOpen_triggered();
  void on_actionSaveAs_triggered();
  void on_actionReport_triggered();
  void on_actionDisplayReport_triggered();
  void onPropertyChanged(task_id taskId, const QString& sPropertyName, const QString& sValue);
  void onTaskRemoved(task_id parentTaskId, task_id childTaskId);
  void onTaskAdded(task_id parentTaskId, task_id childTaskId);  
  void onNewTaskAccepted();
  void createNewSubTask(task_id taskId);
  void onNewSubTaskAccepted();
  void setAutoSortEnabled(group_id);
  void setAutoSortDisabled(group_id);
  void onSortGroupTriggered(int iGroupId);
  void sortGroup(group_id groupId);
  void sortGroups();
  void onTaskDeleted(task_id id);
  void onDocumentModified();

signals:
  void timeTrackingStopped(task_id taskId);
  void documentModified();

private:
  Ui::MainWindow *ui;
  QString m_sFileName;
  std::map<group_id, QPointer<GroupWidget>> m_groupWidgets;
  std::map<task_id, QPointer<TaskWidget>> m_taskWidgets;
  Manager* m_pManager = nullptr;
  TaskCreationDialog* m_pTaskCreationDialog = nullptr;
  std::map<group_id, QTimer*> m_autoSortTimers;
  QSignalMapper* m_pTimeoutGroupIdMapper;
};

#endif // MAINWINDOW_H
