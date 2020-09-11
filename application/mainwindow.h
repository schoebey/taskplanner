#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "id_types.h"
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
class MainWindow : public QMainWindow
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
  void createNewTask(group_id groupId);
  void renameGroup(group_id id, const QString& sNewName);
  void renameTask(task_id id, const QString& sNewName);
  void changeTaskDescription(task_id id, const QString& sNewDescr);
  void onTaskMoved(task_id id, group_id groupId, int iPos);
  void onTaskMoved(task_id id, task_id newParentTaskId, int iPos);
  void onPropertyChanged(task_id taskId, const QString& sPropertyName, const QString& sValue);
  void onPropertyRemoved(task_id taskId, const QString& sPropertyName);
  void onLinkAdded(task_id taskId, QUrl url);
  void onLinkRemoved(task_id taskId, QUrl url);
  void onLinkInserted(task_id taskId, QUrl url, int iPos);
  void onTaskDeleted(task_id id);
  void onTaskRemoved(task_id parentTaskId, task_id childTaskId);
  void onTaskAdded(task_id parentTaskId, task_id childTaskId);
  void onNewTaskAccepted();
  void setPriority(int iPriority);
  void createNewSubTask();
  void createNewSubTask(task_id taskId);
  void onNewSubTaskAccepted();
  void reloadStylesheet(const QString& sPath);
  void initTaskUi();
  void updateTaskUi();
  void startTimeTracking(task_id taskId);
  void stopTimeTracking(task_id taskId);
  void on_actionOpen_triggered();
  void on_actionSave_triggered();
  void on_actionSaveAs_triggered();
  void on_actionReport_triggered();
  void on_actionDisplayReport_triggered();
  void setAutoSortEnabled(group_id);
  void setAutoSortDisabled(group_id);
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
  void onPriorityUpdateRequested(task_id);
  void onAddTimeToTaskRequested(task_id);
  void onRemoveTimeFromTaskRequested(task_id);
  void onAddTagRequested(task_id taskId, tag_id tagId);
  void onRemoveTagRequested(task_id taskId, tag_id tagId);

signals:
  void timeTrackingStopped(task_id taskId);
  void documentModified();

private:
  void timerEvent(QTimerEvent* pEvent) override;
  void closeEvent(QCloseEvent *) override;
  bool eventFilter(QObject *, QEvent *pEvent) override;
  void updateAutoPrioritiesInTaskWidgets();
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
