#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "id_types.h"
#include "taskcontrollerinterface.h"
#include "groupcontrollerinterface.h"

#include <QMainWindow>
#include <QPointer>
#include <QUrl>
#include <QDateTime>
#include <QUndoStack>
#include <map>
#include <memory>
#include <QMessageBox>

namespace Ui {
  class MainWindow;
}

class GroupWidget;
class TaskWidget;
class Manager;
class WidgetManager;
class TaskCreationDialog;
class QSignalMapper;
class SearchFrame;
class SearchController;
class QFileSystemWatcher;
class ToolBarInfoDisplay;
class MainWindow : public QMainWindow, public ITaskController, public IGroupController
{
  Q_OBJECT

public:
  explicit MainWindow(Manager* pManager, QWidget *parent = nullptr);
  ~MainWindow();


  bool loadFile(const QString& sFileName, QString* psErrorMessage = nullptr);

  bool loadMostRecentFile();

  bool saveFile(const QString& sFileName, QString* psErrorMessage = nullptr);

  void restoreDefaultLayout();

private slots:
  void createNewTask(group_id groupId) override;
  void renameGroup(group_id id, const QString& sNewName) override;
  void renameTask(task_id id, const QString& sNewName) override;
  void changeTaskDescription(task_id id, const QString& sNewDescr) override;
  void onTaskMoved(task_id id, group_id groupId, int iPos) override;
  void onTaskMoved(task_id id, task_id newParentTaskId, int iPos) override;
  void onPropertyChanged(task_id taskId, const QString& sPropertyName, const QString& sValue) override;
  void onPropertyRemoved(task_id taskId, const QString& sPropertyName) override;
  void onLinkAdded(task_id taskId, QUrl url) override;
  void onLinkRemoved(task_id taskId, QUrl url) override;
  void onLinkInserted(task_id taskId, QUrl url, int iPos) override;
  void onTaskDeleted(task_id id) override;
  void onTaskRemoved(task_id parentTaskId, task_id childTaskId) override;
  void onTaskAdded(task_id parentTaskId, task_id childTaskId);
  void onNewTaskAccepted();
  void setPriority(int iPriority);
  void createNewSubTask();
  void createNewSubTask(task_id taskId) override;
  void onNewSubTaskAccepted();
  void reloadStylesheet(const QString& sPath);
  void initTaskUi();
  void updateTaskUi();
  void startTimeTracking(task_id taskId) override;
  void stopTimeTracking(task_id taskId) override;
  void on_actionOpen_triggered();
  void on_actionSave_triggered();
  void on_actionSaveAs_triggered();
  void on_actionReport_triggered();
  void on_actionDisplayReport_triggered();
  void setAutoSortEnabled(group_id) override;
  void setAutoSortDisabled(group_id) override;
  void onSortGroupTriggered(int iGroupId);
  void sortGroup(group_id groupId);
  void sortGroups();
  void onDocumentModified();
  void onPasteFromClipboard();
  void onReloadDocument();
  void on_actionAbout_triggered();
  void onWokeUpFromHibernation(const QDateTime& sleepTime,
                               const QDateTime& wakeUpTime);
  void on_actionFind_triggered();
  void onPriorityUpdateRequested(task_id) override;
  void onAddTimeToTaskRequested(task_id) override;
  void onRemoveTimeFromTaskRequested(task_id) override;
  void onTimeTrackingStopped(task_id) override;
  void onChildPropertyChangeRequested(task_id, const QString&, const QString&, bool) override;

signals:
  void timeTrackingStopped(task_id taskId);
  void documentModified();

private:
  void closeEvent(QCloseEvent *) override;
  bool eventFilter(QObject *, QEvent *pEvent) override;
  void saveTempFile();
  void loadPlugins(const QString &sInitialSearchPath = QString());
  void saveSettings();
  void loadSettings();
  void onChooseStylesheet();
  void onChooseScript();
  void showError(const QString& sErrorMessage);
  QMessageBox::StandardButton askSave();

private:
  Ui::MainWindow *ui;
  QString m_sFileName;
  QString m_sStylesheetPath;
  QDateTime m_lastSaveTime;
  Manager* m_pManager = nullptr;
  WidgetManager* m_pWidgetManager = nullptr;
  TaskCreationDialog* m_pTaskCreationDialog = nullptr;
  std::map<group_id, QTimer*> m_autoSortTimers;
  QSignalMapper* m_pTimeoutGroupIdMapper;
  QUndoStack m_undoStack;
  std::vector<std::shared_ptr<QObject>> m_vspPlugins;
  QAction* m_pEnableHibernationDetection;
  QFileSystemWatcher* m_pWatcher;
  ToolBarInfoDisplay* m_pInfoDisplay;
  SearchFrame* m_pSearchFrame;
  std::shared_ptr<SearchController> m_spSearchController;
  void setCurrentFileName(const QString& sFileName);
};

#endif // MAINWINDOW_H
