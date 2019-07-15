#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "groupwidget.h"
#include "taskwidget.h"
#include "widgetmanager.h"
#include "manager.h"
#include "groupinterface.h"
#include "taskinterface.h"
#include "serializerfactory.h"
#include "reportfactory.h"
#include "property.h"
#include "overlaywidget.h"
#include "taskcreationdialog.h"
#include "aboutdialog.h"
#include "hibernationdetector.h"
#include "toolbarinfodisplay.h"

#include "commands/changetaskpropertycommand.h"
#include "commands/changegrouppropertycommand.h"
#include "commands/movetaskcommand.h"
#include "commands/addtaskcommand.h"
#include "commands/deletetaskcommand.h"
#include "commands/addsubtaskcommand.h"


#include <QFileSystemWatcher>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QSignalMapper>
#include <QClipboard>
#include <QThread>
#include <QPluginLoader>
#include <plugininterface.h>

#include <QMouseEvent>
#include <QSettings>
#include <array>
#include <future>
#include <memory>


namespace
{
  QString tempFileNameFromFileName(const QString& sFileName)
  {
    QFileInfo info(sFileName);
    QString sPath = info.path() + "/~" + info.fileName();
    return sPath;
  }
}


Q_DECLARE_METATYPE(QIODevice*)

MainWindow::MainWindow(Manager* pManager, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_pManager(pManager),
  m_pWidgetManager(new WidgetManager(m_pManager, this)),
  m_pTimeoutGroupIdMapper(new QSignalMapper(this))
{
#ifndef Q_OS_MAC
  setWindowFlags(Qt::FramelessWindowHint);
#endif

  ui->setupUi(this);

  qApp->installEventFilter(this);

  auto pUndoAction = m_undoStack.createUndoAction(this);
  pUndoAction->setShortcut(Qt::CTRL + Qt::Key_Z);
  pUndoAction->setIcon(QIcon(":/icons/undo.png"));
  ui->pMainToolBar->insertAction(ui->actionReport, pUndoAction);
  ui->menuEdit->addAction(pUndoAction);

  auto pRedoAction = m_undoStack.createRedoAction(this);
  pRedoAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
  pRedoAction->setIcon(QIcon(":/icons/redo.png"));
  ui->pMainToolBar->insertAction(ui->actionReport, pRedoAction);
  ui->menuEdit->addAction(pRedoAction);

  ui->pMainToolBar->insertSeparator(ui->actionReport);
  ui->pMainToolBar->insertAction(ui->actionReport, ui->actionDisplayReport);


  QWidget* pInfoDisplay = new ToolBarInfoDisplay(this);
  auto pInfoDisplayAction = ui->pInfoToolBar->addWidget(pInfoDisplay);

  QWidget* pSpacer = new QWidget();
  pSpacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
  ui->pInfoToolBar->insertWidget(pInfoDisplayAction, pSpacer);

  connect(m_pTimeoutGroupIdMapper, SIGNAL(mapped(int)), this, SLOT(onSortGroupTriggered(int)));

  QFileSystemWatcher* pWatcher = new QFileSystemWatcher(this);
  pWatcher->addPath("application/resources/stylesheet.css");
  connect(pWatcher, SIGNAL(fileChanged(QString)), this, SLOT(reloadStylesheet(QString)));

  if (QFileInfo("stylesheet.css").exists())
  {
    reloadStylesheet("stylesheet.css");
    pWatcher->addPath("stylesheet.css");
    connect(pWatcher, SIGNAL(fileChanged(QString)), this, SLOT(reloadStylesheet(QString)));
  }
  else
  {
    reloadStylesheet(":/stylesheet.css");
  }



  static const std::vector<QString> c_vsPluginFolders = { QCoreApplication::applicationDirPath() + "/plugins/serializers",
                                                          QCoreApplication::applicationDirPath() + "/plugins/reports" };
  for (const auto& sPath : c_vsPluginFolders)
  {
    loadPlugins(sPath);
  }

  bool bOk = connect(this, SIGNAL(documentModified()), this, SLOT(onDocumentModified()));
  assert(bOk);


  // TODO: move shortcut to group widget and set the context to 'widget'
  QAction* pPasteFromClipboardAction = new QAction(tr("paste"), this);
  pPasteFromClipboardAction->setShortcut(Qt::CTRL + Qt::Key_V);
  pPasteFromClipboardAction->setShortcutContext(Qt::WindowShortcut);
  connect(pPasteFromClipboardAction, SIGNAL(triggered()), this, SLOT(onPasteFromClipboard()));
  addAction(pPasteFromClipboardAction);

  QAction* pReloadAction = new QAction(tr("reload current document"), this);
  pReloadAction->setShortcut(Qt::CTRL + Qt::Key_R);
  connect(pReloadAction, SIGNAL(triggered()), this, SLOT(onReloadDocument()));
  addAction(pReloadAction);

  initTaskUi();

  startTimer(3000);

  QMenu* pMenuOptions = new QMenu(tr("Options"), ui->menuTools);
  ui->menuTools->addMenu(pMenuOptions);


  auto pDetector = new HibernationDetector(this);
  bOk = connect(pDetector, SIGNAL(wokeUpFromHibernation(QDateTime, QDateTime)),
                this, SLOT(onWokeUpFromHibernation(QDateTime, QDateTime)));
  assert(bOk);
  Q_UNUSED(bOk)


  m_pEnableHibernationDetection = new QAction(tr("hibernation detection"), this);
  m_pEnableHibernationDetection->setCheckable(true);
  connect(m_pEnableHibernationDetection, &QAction::toggled, pDetector, &HibernationDetector::setEnabled);
  pMenuOptions->addAction(m_pEnableHibernationDetection);



  loadSettings();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent* /*event*/)
{
  saveFile(m_sFileName);
  saveSettings();
}



void MainWindow::saveSettings()
{
  // write window state, currently loaded file etc. to a settings file
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION_NAME, APP_NAME);

  settings.beginGroup("window");
  settings.setValue("state", saveState());
  settings.setValue("geometry", saveGeometry());
  settings.endGroup();

  settings.beginGroup("files");
  settings.setValue("mostRecentFile", m_sFileName);
  settings.endGroup();

  settings.beginGroup("options");
  settings.setValue("hibernationDetection", m_pEnableHibernationDetection->isChecked());
  settings.endGroup();
}

void MainWindow::loadSettings()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION_NAME, APP_NAME);

  settings.beginGroup("window");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("state").toByteArray());
  settings.endGroup();

  settings.beginGroup("files");
  m_sFileName = settings.value("mostRecentFile").toString();
  settings.endGroup();

  settings.beginGroup("options");
  m_pEnableHibernationDetection->setChecked(settings.value("hibernationDetection", true).toBool());
  settings.endGroup();
}

void MainWindow::initTaskUi()
{
  m_pWidgetManager->clear();

  updateTaskUi();
}


void MainWindow::updateTaskUi()
{
  bool bOk = disconnect(this, SIGNAL(documentModified()), this, SLOT(onDocumentModified()));
  assert(bOk);


  for (const auto& groupId : m_pManager->groupIds())
  {
    IGroup* pGroup = m_pManager->group(groupId);

    if (nullptr != pGroup)
    {
      GroupWidget* pGroupWidget = m_pWidgetManager->groupWidget(groupId);
      if (nullptr == pGroupWidget)
      {
        pGroupWidget = m_pWidgetManager->createGroupWidget(groupId);

        QHBoxLayout* pLayout = dynamic_cast<QHBoxLayout*>(ui->frame->layout());
        pLayout->addWidget(pGroupWidget);
      }


      std::map<int, std::vector<ITask*>> tasksByPriority;
      for (const auto& taskId : pGroup->taskIds())
      {
        ITask* pTask = m_pManager->task(taskId);
        if (nullptr != pTask)
        {
          tasksByPriority[pTask->priority().priority(0)].push_back(pTask);
        }
      }


      for (const auto& task : tasksByPriority)
      {
        for (const auto& pTask : task.second)
        {
          TaskWidget* pTaskWidget = m_pWidgetManager->taskWidget(pTask->id());
          if (nullptr == pTaskWidget)
          {
            pTaskWidget = m_pWidgetManager->createTaskWidget(pTask->id());

            auto pParentTask = m_pManager->task(pTask->parentTask());
            if (nullptr == pParentTask)
            {
              pGroupWidget->insertTask(pTaskWidget);
            }
          }
        }
      }


      // afer all the tasks have been created, the group properties can be set
      for (auto groupId : m_pManager->groupIds())
      {
        pGroupWidget = m_pWidgetManager->groupWidget(groupId);
        IGroup* pGroup = m_pManager->group(groupId);

        if (nullptr != pGroup &&
            nullptr != pGroupWidget)
        {
          bool bOk(false);
          bool bAutoSortingEnabled = conversion::fromString<bool>(pGroup->propertyValue("autoSorting"), bOk);
          if (bOk)  { pGroupWidget->setAutoSortingEnabled(bAutoSortingEnabled); }
        }
      }
    }
  }


  // after having created all task widgets, build the hierarchy
  for (const auto& taskId : m_pManager->taskIds())
  {
    auto pTask = m_pManager->task(taskId);
    if (nullptr != pTask)
    {
      TaskWidget* pTaskWidget = m_pWidgetManager->taskWidget(taskId);
      if (nullptr != pTaskWidget)
      {
        TaskWidget* pParentTaskWidget = m_pWidgetManager->taskWidget(pTask->parentTask());
        if (nullptr != pParentTaskWidget)
        {
          pParentTaskWidget->insertTask(pTaskWidget);
        }
      }
    }
  }


  // reconnect to the modified signal so that we get all the modifications from now on
  bOk = connect(this, SIGNAL(documentModified()), this, SLOT(onDocumentModified()));
  assert(bOk);


  // update the auto priority in all widgets (old and new)
  updateAutoPrioritiesInTaskWidgets();
}

void MainWindow::loadPlugins(const QString& sInitialSearchPath)
{
  QString sSearchPath = sInitialSearchPath;

  QDir d(sSearchPath);

  // since providing a filter breaks entryInfoList
  // (returned list is empty as soon as a filter is set. Qt-Bug in 5.6?)
  // we have to filter out symlinks manually...
  for (const auto& el : d.entryInfoList())
  {
    if (el.isSymLink())  { continue; }
    else if (el.isDir())
    {
      QDir subfolder(el.filePath());
      if (subfolder.dirName() != "." &&
          subfolder.dirName() != "..")
      {
        loadPlugins(el.filePath());
      }
    }
    else {
      QPluginLoader loader(el.filePath());

      QObject* pPlugin = loader.instance();
      if (nullptr != pPlugin)
      {
        IPlugin* p = dynamic_cast<IPlugin*>(pPlugin);
        if (nullptr != p)
        {
          p->initialize();
          m_vspPlugins.push_back(std::shared_ptr<QObject>(pPlugin));
        }
      }
    }
  }
}

void MainWindow::reloadStylesheet(const QString& sPath)
{
  qDebug() << sPath;
  QFile f(sPath);
  if (f.open(QIODevice::ReadOnly))
  {
    qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
  }
}

void MainWindow::saveTempFile()
{
  QString sPath = tempFileNameFromFileName(m_sFileName);
  saveFile(sPath);
}

void MainWindow::onDocumentModified()
{
  setWindowModified(true);

  // save modifications to a temp file
  auto fnSave = [&]()
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    saveTempFile();
  };

  std::thread{fnSave}.detach();
}

void MainWindow::createNewTask(group_id groupId)
{
  IGroup* pGroup = m_pManager->group(groupId);
  if (nullptr != pGroup)
  {
    GroupWidget* pGroupWidget = m_pWidgetManager->groupWidget(groupId);
    if (nullptr != pGroupWidget)
    {
      delete m_pTaskCreationDialog;
      m_pTaskCreationDialog = new TaskCreationDialog(this);
      m_pTaskCreationDialog->setProperty("groupId", int(groupId));
      connect(m_pTaskCreationDialog, SIGNAL(accepted()), this, SLOT(onNewTaskAccepted()));
      m_pTaskCreationDialog->show();
    }
  }
}

void MainWindow::onNewTaskAccepted()
{
  group_id groupId = m_pTaskCreationDialog->property("groupId").toInt();

  IGroup* pGroup = m_pManager->group(groupId);
  GroupWidget* pGroupWidget = m_pWidgetManager->groupWidget(groupId);
  if (nullptr != pGroup &&
      nullptr != pGroupWidget)
  {
    AddTaskCommand* pCommand = new AddTaskCommand(groupId,
                                                  m_pTaskCreationDialog->name(),
                                                  m_pTaskCreationDialog->description(),
                                                  m_pManager, m_pWidgetManager);
    m_undoStack.push(pCommand);

    emit documentModified();
  }
}

void MainWindow::createNewSubTask(task_id taskId)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask)
  {
    TaskWidget* pTaskWidget = m_pWidgetManager->taskWidget(taskId);
    if (nullptr != pTaskWidget)
    {
      delete m_pTaskCreationDialog;
      m_pTaskCreationDialog = new TaskCreationDialog(this);
      m_pTaskCreationDialog->setProperty("taskId", int(taskId));
      connect(m_pTaskCreationDialog, SIGNAL(accepted()), this, SLOT(onNewSubTaskAccepted()));
      m_pTaskCreationDialog->show();
    }
  }
}

void MainWindow::onNewSubTaskAccepted()
{
  task_id parentTaskId = m_pTaskCreationDialog->property("taskId").toInt();

  ITask* pParentTask = m_pManager->task(parentTaskId);
  if (nullptr != pParentTask)
  {

    AddSubTaskCommand* pCommand = new AddSubTaskCommand(parentTaskId,
                                                        m_pTaskCreationDialog->name(),
                                                        m_pTaskCreationDialog->description(),
                                                        m_pManager, m_pWidgetManager);
    m_undoStack.push(pCommand);

    emit documentModified();
  }
}

void MainWindow::renameGroup(group_id id, const QString& sNewName)
{
  IGroup* pGroup = m_pManager->group(id);
  if (nullptr != pGroup)
  {
    ChangeGroupPropertyCommand* pChangeCommand =
        new ChangeGroupPropertyCommand(id, "name", pGroup->name(), sNewName,
                                        m_pManager, m_pWidgetManager);
    m_undoStack.push(pChangeCommand);

    emit documentModified();
  }
}

void MainWindow::renameTask(task_id id, const QString& sNewName)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    ChangeTaskPropertyCommand* pChangeCommand =
        new ChangeTaskPropertyCommand(id, "name", pTask->name(), sNewName, m_pManager, m_pWidgetManager);
    m_undoStack.push(pChangeCommand);

    emit documentModified();
  }
}

void MainWindow::changeTaskDescription(task_id id, const QString& sNewDescr)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    ChangeTaskPropertyCommand* pChangeCommand =
        new ChangeTaskPropertyCommand(id, "description", pTask->description(), sNewDescr, m_pManager, m_pWidgetManager);
    m_undoStack.push(pChangeCommand);

    emit documentModified();
  }
}

void MainWindow::onTaskMoved(task_id id, group_id newParentGroupId, int iPos)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    int iOldPos = pTask->priority().priority(0);
    IGroup* pOldGroup = m_pManager->group(pTask->group());
    IGroup* pNewGroup = m_pManager->group(newParentGroupId);

    // only 'move' the task if it really has been moved from one group to another
    if (nullptr != pOldGroup && nullptr != pNewGroup &&
        (pOldGroup != pNewGroup || iOldPos != iPos || -1 != pTask->parentTask()))
    {
      MoveTaskCommand* pCommand = new MoveTaskCommand(id,
                                                      pTask->group(),
                                                      newParentGroupId,
                                                      pTask->parentTask(), -1,
                                                      iOldPos, iPos,
                                                      m_pManager,m_pWidgetManager);
      m_undoStack.push(pCommand);

      emit documentModified();
    }
    else
    {
      auto pGroupWidget = m_pWidgetManager->groupWidget(newParentGroupId);
      if (nullptr != pGroupWidget)
      {
        pGroupWidget->insertTask(m_pWidgetManager->taskWidget(id), iPos);
      }
    }
  }
}

void MainWindow::onTaskMoved(task_id id, task_id newParentTaskId, int iPos)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    int iOldPos = pTask->priority().priority(0);
    ITask* pOldParentTask = m_pManager->task(pTask->parentTask());
    ITask* pNewParentTask = m_pManager->task(newParentTaskId);


    if (pOldParentTask != pNewParentTask || iOldPos != iPos)
    {
//      TODO: test moving tasks to subtasks or between subtasks or reordering of subtasks
      MoveTaskCommand* pCommand = new MoveTaskCommand(id,
                                                      pTask->group(),
                                                      nullptr != pNewParentTask ? pNewParentTask->group() : -1,
                                                      pTask->parentTask(),
                                                      nullptr != pNewParentTask ? pNewParentTask->id() : -1,
                                                      iOldPos, iPos,
                                                      m_pManager,m_pWidgetManager);
      m_undoStack.push(pCommand);

      emit documentModified();
    }
    else
    {
      auto pTaskWidget = m_pWidgetManager->taskWidget(newParentTaskId);
      if (nullptr != pTaskWidget)
      {
        pTaskWidget->insertTask(m_pWidgetManager->taskWidget(id), iPos);
      }
    }
  }
}

void MainWindow::startTimeTracking(task_id taskId)
{
  for (const auto& otherTaskId : m_pManager->taskIds())
  {
    if (otherTaskId != taskId)
    {
      ITask* pTask = m_pManager->task(otherTaskId);
      if (nullptr != pTask)
      {
        pTask->stopWork();
        emit timeTrackingStopped(otherTaskId);
      }

    }
  }


  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask)
  {
    pTask->startWork();
  }

  emit documentModified();
}

void MainWindow::stopTimeTracking(task_id taskId)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask)
  {
    pTask->stopWork();
    emit timeTrackingStopped(taskId);

    emit documentModified();
  }
}

void MainWindow::restoreDefaultLayout()
{
  if (0 == m_pManager->groupIds().size())
  {
    IGroup* pGroup = m_pManager->addGroup();
    pGroup->setName("backlog");

    pGroup = m_pManager->addGroup();
    pGroup->setName("in progress");

    pGroup = m_pManager->addGroup();
    pGroup->setName("done");

    initTaskUi();
  }
}

bool MainWindow::loadFile(const QString& sFileName, QString* psErrorMessage)
{
  if (!sFileName.isEmpty())
  {
    QFileInfo info(sFileName);
    QString sSuffix = info.suffix();

    auto serializers = SerializerFactory::classes();
    auto it = std::find_if(serializers.begin(), serializers.end(),
                           [sSuffix](const std::pair<QString, QString>& p)
    { return p.second == sSuffix; });

    if (it != serializers.end())
    {
      tspSerializer spReader = SerializerFactory::create(it->first);
      if (!spReader->setParameter("fileName", sFileName))
      {
        if (nullptr != psErrorMessage)
        {
          *psErrorMessage = tr("parameter 'fileName' not supported "
                               "for serializer '%1'").arg(it->first);
        }

        return false;
      }

      EDeserializingError de = m_pManager->deserializeFrom(spReader.get());


      if (EDeserializingError::eOk == de)
      {
        m_sFileName = sFileName;

        setWindowTitle(QString("%1[*] - %2").arg(m_sFileName).arg(QGuiApplication::applicationDisplayName()));

        initTaskUi();

        setWindowModified(false);

        return true;
      }
      else
      {
        if (nullptr != psErrorMessage)
        {
          switch (de)
          {
          case EDeserializingError::eInternalError:
            *psErrorMessage = tr("internal error");
            break;
          case EDeserializingError::eResourceError:
            *psErrorMessage = tr("resource error");
            break;
          case EDeserializingError::eWrongParameter:
            *psErrorMessage = tr("wrong parameter");
            break;
          case EDeserializingError::eWrongFormat:
            *psErrorMessage = tr("wrong format. Maybe not a %1 file?").arg(it->first);
            break;
          case EDeserializingError::eOk:
          default:
            break;
          }
        }
      }
    }
  }

  return false;
}

bool MainWindow::loadMostRecentFile()
{
  return loadFile(m_sFileName);
}


bool MainWindow::saveFile(const QString& sFileName, QString* psErrorMessage)
{
  if (!sFileName.isEmpty())
  {
    QFileInfo info(sFileName);
    QString sSuffix = info.suffix();

    auto serializers = SerializerFactory::classes();
    auto it = std::find_if(serializers.begin(), serializers.end(),
                           [sSuffix](const std::pair<QString, QString>& p)
    { return p.second == sSuffix; });
    if (it != serializers.end())
    {
      tspSerializer spWriter = SerializerFactory::create(it->first);
      if (!spWriter->setParameter("fileName", sFileName))
      {
        if (nullptr != psErrorMessage)
        {
          *psErrorMessage = tr("parameter 'fileName' not supported "
                               "for serializer '%1'").arg(it->first);
        }

        return false;
      }

      QString sErrorMessage;
      ESerializingError err = m_pManager->serializeTo(spWriter.get());
      switch (err)
      {
      case ESerializingError::eInternalError:
        sErrorMessage = tr("serialisation to '%1' has failed with an internal error.")
            .arg(sFileName);
        break;
      case ESerializingError::eResourceError:
        sErrorMessage = tr("serialisation to '%1' has failed with a resource error.")
            .arg(sFileName);
        break;
      case ESerializingError::eWrongParameter:
        sErrorMessage = tr("serialisation to '%1' has failed. Wrong/missing parameter.")
            .arg(sFileName);
        break;
      case ESerializingError::eOk:
      default:
        break;
      }

      if (nullptr != psErrorMessage)
      {
        *psErrorMessage = sErrorMessage;
      }

      // only if the saved document is the currently loaded one,
      // reset the modified flag and update the saved date
      if (QThread::currentThread() == thread() &&
          sFileName == m_sFileName)
      {
        setWindowModified(false);
        m_lastSaveTime = QDateTime::currentDateTime();
      }


      return ESerializingError::eOk == err;
    }
  }

  return false;
}

void MainWindow::on_actionOpen_triggered()
{
  QString sFileName = QFileDialog::getOpenFileName(this, tr("Open task file..."));
  if (sFileName.isEmpty())  { return; }

  if (isWindowModified())
  {
    auto msecs = m_lastSaveTime.msecsTo(QDateTime::currentDateTime());
    auto secs = msecs / 1000;
    auto mins = secs / 60;
    auto hours = mins / 60;
    QString sLastSave;
    if (0 < hours)  { sLastSave = QString("%1 Stunde(n)").arg(hours);}
    if (0 < mins)  { sLastSave = QString("%1 Minute(n)").arg(mins);}
    if (0 < secs)  { sLastSave = QString("%1 Sekunde(n)").arg(secs);}
    QString sLastSaveMessage;
    if (m_lastSaveTime.isValid())
    {
      sLastSaveMessage = tr("The document has been saved %1 ago.").arg(sLastSave);
    }
    else
    {
      sLastSaveMessage = "The document hasn't been saved yet.";
    }

    auto button =
        QMessageBox::question(this, tr("Save modifications?"),
                              tr("Should the new document be opened?\n"
                                 "There are unsaved modifications.\n\n"
                                 "%1").arg(sLastSaveMessage));
    if (QMessageBox::No == button) { return; }
  }


  // check the filesystem for a temp file of the same name with newer timestamp
  QFileInfo info(sFileName);
  QFileInfo tempInfo(tempFileNameFromFileName(sFileName));
  if (info.exists() && tempInfo.exists() &&
      tempInfo.lastModified() > info.lastModified())
  {
    QMessageBox mb(QMessageBox::Question, tr("Load temporary file?"),
                   tr("There's a temporary file of the selected file "
                      "that is newer than the file itself.\n\n"
                      "Should the temporary file be loaded instead?"),
                   QMessageBox::Yes | QMessageBox::No);
    mb.setDetailedText(tr("%1 saved on %2,\n"
                          "%3 saved on %4\n\n")
                       .arg(info.fileName()).arg(info.lastModified().toString("yyyy-MM-dd hh:mm:ss"))
                       .arg(tempInfo.fileName()).arg(tempInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss")));


    if (QMessageBox::Yes == mb.exec())
    {
      sFileName = tempInfo.filePath();
    }
  }


  QString sErrorMessage;
  if (!loadFile(sFileName, &sErrorMessage))
  {
    QMessageBox::critical(this, tr("error loading file"), sErrorMessage);
  }
}

void MainWindow::on_actionSave_triggered()
{
  QString sErrorMessage;
  if (!saveFile(m_sFileName, &sErrorMessage))
  {
    QMessageBox::critical(this, tr("error writing file"), sErrorMessage);
  }
}

void MainWindow::on_actionSaveAs_triggered()
{
  QStringList list;
  auto serializers = SerializerFactory::classes();
  for (const auto& element : serializers)
  {
    list.append(QString("%1 (*.%2)").arg(element.first).arg(element.second));
  }

  QString sFilter = list.join(";;");
  QString sSelectedFilter;
  QString sFileName = QFileDialog::getSaveFileName(this, tr("Save task file as..."),
                                                   QString(), sFilter,
                                                   &sSelectedFilter);
  if (sFileName.isEmpty())  { return; }

  QString sErrorMessage;
  if (!saveFile(sFileName, &sErrorMessage))
  {
    QMessageBox::critical(this, tr("error writing file"), sErrorMessage);
  }
}

void MainWindow::on_actionDisplayReport_triggered()
{
  tspReport spReport = ReportFactory::create("text");
  if (nullptr != spReport)
  {
    QByteArray ba;
    QBuffer buffer(&ba);
    spReport->setParameter("device", QVariant::fromValue<QIODevice*>(&buffer));
    spReport->create(*m_pManager);

    QString s(ba.data());

    OverlayWidget* pOverlay = new OverlayWidget(this);
    QLabel* pLabel = new QLabel(s);
    pLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    pLabel->setObjectName("report");
    pLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    pOverlay->setAutoDeleteOnClose(true);
    pOverlay->addWidget(pLabel);
    pOverlay->setTitle(tr("Report"));
    pLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    pOverlay->appear();
  }
}

void MainWindow::on_actionReport_triggered()
{
  auto supportedReports = ReportFactory::classes();

  QStringList list;
  for (const auto& element : supportedReports)
  {
    list.append(QString("%1 (*.%2)").arg(element.first).arg(element.second));
  }

  QString sFilter = list.join(";;");
  QString sSelectedFilter;
  QString sFileName = QFileDialog::getSaveFileName(this, tr("save report as..."),
                                                   QString(), sFilter, &sSelectedFilter);
  if (!sFileName.isEmpty())
  {
    auto it = std::find_if(supportedReports.begin(), supportedReports.end(),
                           [sSelectedFilter](const std::pair<QString, QString>& p)
    { return QString("%1 (*.%2)").arg(p.first).arg(p.second) == sSelectedFilter; });
    if (it != supportedReports.end())
    {
      tspReport spReport = ReportFactory::create(it->first);
      if (nullptr != spReport)
      {
        QFile f(sFileName);
        if (f.open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
          spReport->setParameter("device", QVariant::fromValue<QIODevice*>(&f));
          spReport->setParameter("fileName", sFileName);
          spReport->create(*m_pManager);
          f.close();
        }
      }
    }
  }
}

void MainWindow::on_actionAbout_triggered()
{
  AboutDialog* pAboutDialog = new AboutDialog(this);
  pAboutDialog->setAutoDeleteOnClose(true);
  pAboutDialog->appear();
}

void MainWindow::onPropertyChanged(task_id taskId,
                                   const QString& sPropertyName,
                                   const QString& sValue)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask)
  {
    QString sOldValue = pTask->propertyValue(sPropertyName);
    if (sOldValue == sValue)  { return; }

    bool bNewValueAccepted = pTask->setPropertyValue(sPropertyName, sValue);


    TaskWidget* pTaskWidget = m_pWidgetManager->taskWidget(taskId);
    if (nullptr != pTaskWidget)
    {
      if (bNewValueAccepted)
      {
        ChangeTaskPropertyCommand* pChangeCommand =
            new ChangeTaskPropertyCommand(taskId, sPropertyName, sOldValue, sValue, m_pManager, m_pWidgetManager);
        m_undoStack.push(pChangeCommand);
      }

      pTaskWidget->setHighlight(pTaskWidget->highlight() |
                               (bNewValueAccepted ? EHighlightMethod::eValueAccepted :
                                                   EHighlightMethod::eValueRejected));
      pTaskWidget->setPropertyValue(sPropertyName, pTask->propertyValue(sPropertyName));

      if ("color" == sPropertyName)
      {
        bool bOk(false);
        auto color = conversion::fromString<QColor>(pTask->propertyValue("color"), bOk);
        if (bOk)
        {
          pTaskWidget->setOverlayBackground(color);
        }
      }
    }

    auto itTimer = m_autoSortTimers.find(pTask->group());
    if (itTimer != m_autoSortTimers.end() &&
        nullptr != itTimer->second &&
        itTimer->second->isActive())
    {
      sortGroup(pTask->group());
    }

    emit documentModified();
  }
}

void MainWindow::onPropertyRemoved(task_id taskId, const QString& sPropertyName)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask)
  {
    ChangeTaskPropertyCommand* pChangeCommand =
        new ChangeTaskPropertyCommand(taskId, sPropertyName, pTask->propertyValue(sPropertyName),
                                      QString(), m_pManager, m_pWidgetManager);
    m_undoStack.push(pChangeCommand);
  }
}

void MainWindow::onLinkAdded(task_id taskId, QUrl url)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask)
  {
    bool bOk(false);
    auto links = conversion::fromString<std::vector<QUrl>>(pTask->propertyValue("links"), bOk);
    if (bOk)
    {
      auto it = std::find(links.begin(), links.end(), url);
      if (it == links.end())
      {
        links.push_back(url);
        QUndoCommand* pCommand = new ChangeTaskPropertyCommand(taskId, "links",
                                                               pTask->propertyValue("links"), conversion::toString(links),
                                                               m_pManager, m_pWidgetManager);
        m_undoStack.push(pCommand);
      }
      else
      {
        // link already present - do nothing
      }
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    assert(false);
  }
}

void MainWindow::onLinkRemoved(task_id taskId, QUrl url)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask)
  {
    bool bOk(false);
    auto links = conversion::fromString<std::vector<QUrl>>(pTask->propertyValue("links"), bOk);
    if (bOk)
    {
      auto it = std::find(links.begin(), links.end(), url);
      if (it != links.end())
      {
        links.erase(it);
        QUndoCommand* pCommand = new ChangeTaskPropertyCommand(taskId, "links",
                                                               pTask->propertyValue("links"), conversion::toString(links),
                                                               m_pManager, m_pWidgetManager);
        m_undoStack.push(pCommand);
      }
      else
      {
        // link not present - do nothing
      }
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    assert(false);
  }
}

void MainWindow::onLinkInserted(task_id /*taskId*/, QUrl /*url*/, int /*iPos*/)
{

}

void MainWindow::onTaskAdded(task_id parentTaskId, task_id childTaskId)
{
  ITask* pNewParentTask = m_pManager->task(parentTaskId);
  ITask* pTask = m_pManager->task(childTaskId);
  if (nullptr != pNewParentTask && nullptr != pTask &&
      pTask->parentTask() != parentTaskId)
  {
    bool bOk = false;
    int iPosition = pTask->propertyValue("sort_priority").toInt(&bOk);
    if (!bOk)  { iPosition = -1; }
    QUndoCommand* pCommand =
        new MoveTaskCommand(childTaskId, pTask->group(), pNewParentTask->group(),
                            pTask->parentTask(), parentTaskId,
                            iPosition, iPosition,
                            m_pManager, m_pWidgetManager);


    m_undoStack.push(pCommand);

    emit documentModified();
  }
}

void MainWindow::onTaskRemoved(task_id parentTaskId, task_id childTaskId)
{
  ITask* pTask = m_pManager->task(parentTaskId);
  if (nullptr != pTask)
  {
    pTask->removeTask(childTaskId);

    emit documentModified();
  }
}

void MainWindow::onTaskDeleted(task_id id)
{
  if (nullptr != m_pManager->task(id) &&
      nullptr != m_pWidgetManager->taskWidget(id))
  {
    QUndoCommand* pCommand =
        new DeleteTaskCommand(id, m_pManager, m_pWidgetManager);
    m_undoStack.push(pCommand);

    emit documentModified();
  }
}

void MainWindow::setAutoSortEnabled(group_id groupId)
{
  if (m_autoSortTimers.find(groupId) ==
      m_autoSortTimers.end())
  {
    m_autoSortTimers[groupId] = new QTimer(this);
    m_pTimeoutGroupIdMapper->setMapping(m_autoSortTimers[groupId], int(groupId));
    connect(m_autoSortTimers[groupId], SIGNAL(timeout()), m_pTimeoutGroupIdMapper, SLOT(map()));
  }

  m_autoSortTimers[groupId]->start(60000);

  sortGroup(groupId);

  IGroup* pGroup = m_pManager->group(groupId);
  if (nullptr != pGroup)
  {
    pGroup->setPropertyValue("autoSorting", conversion::toString<bool>(true));
  }

  emit documentModified();
}

void MainWindow::setAutoSortDisabled(group_id groupId)
{
  auto it = m_autoSortTimers.find(groupId);
  if (it != m_autoSortTimers.end())
  {
    it->second->stop();
  }


  IGroup* pGroup = m_pManager->group(groupId);
  if (nullptr != pGroup)
  {
    pGroup->setPropertyValue("autoSorting", conversion::toString<bool>(false));
  }

  emit documentModified();
}

void MainWindow::onSortGroupTriggered(int iGroupId)
{
  sortGroup(static_cast<group_id>(iGroupId));
}

void MainWindow::sortGroup(group_id groupId)
{
  std::map<double, std::vector<task_id>> sortedTaskIds;
  for (const auto& id : m_pManager->taskIds())
  {
    ITask* pTask = m_pManager->task(id);
    if (nullptr != pTask &&
        groupId == pTask->group())
    {
      qDebug() << pTask->autoPriority();
      sortedTaskIds[pTask->autoPriority()].push_back(id);
    }
  }


  GroupWidget* pGroupWidget = m_pWidgetManager->groupWidget(groupId);
  if (nullptr != pGroupWidget)
  {
    std::vector<task_id> vIds;
    for (const auto& el : sortedTaskIds)
    {
      std::vector<task_id> vIdsOfSamePrio;
      for (const auto& id : el.second)
      {
        vIdsOfSamePrio.push_back(id);
      }
      vIds.insert(vIds.begin(), vIdsOfSamePrio.begin(), vIdsOfSamePrio.end());
    }
    pGroupWidget->reorderTasks(vIds);
  }
}

void MainWindow::sortGroups()
{
  for (auto groupId : m_pManager->groupIds())
  {
    sortGroup(groupId);
  }
}

void MainWindow::onPasteFromClipboard()
{
  QClipboard* pClipboard = QApplication::clipboard();
  if (nullptr != pClipboard)
  {
    QString sText = pClipboard->text();
    auto elements = sText.split("\n");

    // check if it's a bullet point list
    QRegExp rx(R"(^(\s*)(\*|\-|\>)+(.*)$)");
    bool bIsBulletPointList = true;
    for (const auto& el : elements)
    {
      bIsBulletPointList |= (0 == rx.indexIn(el));
    }

    if (bIsBulletPointList)
    {
      // group?
      IGroup* pGroup = nullptr;
      for (const auto& gid : m_pManager->groupIds())
      {
        pGroup = m_pManager->group(gid);
        if (nullptr != pGroup) break;
      }
      // iterate through all entries in the list
      // indented entries should become sub-tasks
      int iCurrentIndentationLevel = 0;
      std::map<int, ITask*> potentialParentTasks;

      ITask* pNewTask = nullptr;
      for (const auto& el : elements)
      {
        if (0 == rx.indexIn(el))
        {
          pNewTask = m_pManager->addTask();
          QString sIndent = rx.cap(1);
          iCurrentIndentationLevel = sIndent.size();

          // find potential parent by looking for a task with a lesser indent
          ITask* pPotentialParentTask = nullptr;
          for (const auto& task : potentialParentTasks)
          {
            if (task.first < iCurrentIndentationLevel)  { pPotentialParentTask = task.second; }
          }

          if (nullptr != pPotentialParentTask)
          {
            // Subtask!
            pPotentialParentTask->addTask(pNewTask->id());
          }

          pGroup->addTask(pNewTask->id());

          pNewTask->setName(rx.cap(3));

          // remember latest task of current indent level as potential parent
          potentialParentTasks[iCurrentIndentationLevel] = pNewTask;
        }
        else
        {
          // not a bullet point, so it is considered a description to the previous task
          if (nullptr != pNewTask)
          {
            QString sDesc = pNewTask->description();
            sDesc += (sDesc.isEmpty() ? "" : "\n") + el.trimmed();
            pNewTask->setDescription(sDesc);
          }
        }
      }

      updateTaskUi();
    }
  }
}

void MainWindow::updateAutoPrioritiesInTaskWidgets()
{
  for (const auto& taskId : m_pManager->taskIds())
  {
    auto pTask = m_pManager->task(taskId);
    if (nullptr != pTask)
    {
      TaskWidget* pWidget = m_pWidgetManager->taskWidget(pTask->id());
      if (nullptr != pWidget)
      {
        pWidget->setAutoPriority(pTask->autoPriority());
      }
    }
  }
}

void MainWindow::timerEvent(QTimerEvent* /*pEvent*/)
{
  updateAutoPrioritiesInTaskWidgets();
}

void MainWindow::onReloadDocument()
{
  loadFile(m_sFileName);
}


void MainWindow::onWokeUpFromHibernation(const QDateTime& sleepTime,
                                         const QDateTime& wakeUpTime)
{
  // find all tasks that are currently being tracked
  // create a work interrupt of the duration of the hibernation
  for (const auto& taskId : m_pManager->taskIds())
  {
    ITask* pTask = m_pManager->task(taskId);
    if (nullptr != pTask &&
        pTask->isTrackingTime())
    {
      pTask->stopWork(sleepTime);

      pTask->startWork(wakeUpTime);
    }
  }
}

bool MainWindow::eventFilter(QObject* /*pWatched*/, QEvent* pEvent)
{
  if (pEvent->type() == QEvent::MouseMove)
  {
    QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
    if (nullptr != pMouseEvent)
    {
      QPoint ptGlobal = pMouseEvent->globalPos();
      for (const auto& groupId : m_pManager->groupIds())
      {
        auto pGroup = m_pWidgetManager->groupWidget(groupId);
        if (nullptr != pGroup)
        {
          QPoint pt = pGroup->mapFromGlobal(ptGlobal);
          if (pGroup->rect().contains(pt))
          {
            pGroup->onMouseMoved(pt);
          }
        }
      }
    }
  }

  return false;
}
