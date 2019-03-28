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

#include "commands/changetaskpropertycommand.h"
#include "commands/changegrouppropertycommand.h"
#include "commands/movetaskcommand.h"


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

#include <array>
#include <future>

Q_DECLARE_METATYPE(QIODevice*)

MainWindow::MainWindow(Manager* pManager, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_pManager(pManager),
  m_pWidgetManager(new WidgetManager(m_pManager, this)),
  m_pTimeoutGroupIdMapper(new QSignalMapper(this))
{
  ui->setupUi(this);


  ui->toolBar->addAction(m_undoStack.createUndoAction(this));
  ui->toolBar->addAction(m_undoStack.createRedoAction(this));

  connect(m_pTimeoutGroupIdMapper, SIGNAL(mapped(int)), this, SLOT(onSortGroupTriggered(int)));

  QFileSystemWatcher* pWatcher = new QFileSystemWatcher(this);
  pWatcher->addPath("gui/resources/stylesheet.css");
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

  bool bOk = connect(this, SIGNAL(documentModified()), this, SLOT(onDocumentModified()));
  assert(bOk);
  Q_UNUSED(bOk)


  QAction* pPasteFromClipboardAction = new QAction(tr("paste"), this);
  pPasteFromClipboardAction->setShortcut(Qt::CTRL + Qt::Key_V);
  pPasteFromClipboardAction->setShortcutContext(Qt::WindowShortcut);
  connect(pPasteFromClipboardAction, SIGNAL(triggered()), this, SLOT(onPasteFromClipboard()));
  addAction(pPasteFromClipboardAction);

  initTaskUi();

  startTimer(3000);
}

MainWindow::~MainWindow()
{
  saveFile(m_sFileName);
  delete ui;
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
          pParentTaskWidget->addTask(pTaskWidget);
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
  QFileInfo info(m_sFileName);
  QString sPath = info.path() + "/~" + info.fileName();
  saveFile(sPath);
}

void MainWindow::onDocumentModified()
{
  setWindowModified(true);

  // save modifications to a temp file
  auto f = std::async(std::launch::async, &MainWindow::saveTempFile, this);
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
      m_pTaskCreationDialog->setProperty("groupId", groupId);
      connect(m_pTaskCreationDialog, SIGNAL(accepted()), this, SLOT(onNewTaskAccepted()));
      m_pTaskCreationDialog->show();
    }
  }
}

void MainWindow::onNewTaskAccepted()
{
  group_id groupId = m_pTaskCreationDialog->property("groupId").value<group_id>();

  IGroup* pGroup = m_pManager->group(groupId);
  GroupWidget* pGroupWidget = m_pWidgetManager->groupWidget(groupId);
  if (nullptr != pGroup &&
      nullptr != pGroupWidget)
  {
    ITask* pTask = m_pManager->addTask();
    pTask->setName(m_pTaskCreationDialog->name());
    pTask->setDescription(m_pTaskCreationDialog->description());
    pGroup->addTask(pTask->id());
    pGroupWidget->insertTask(m_pWidgetManager->createTaskWidget(pTask->id()));

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
      m_pTaskCreationDialog->setProperty("taskId", taskId);
      connect(m_pTaskCreationDialog, SIGNAL(accepted()), this, SLOT(onNewSubTaskAccepted()));
      m_pTaskCreationDialog->show();
    }
  }
}

void MainWindow::onNewSubTaskAccepted()
{
  task_id taskId = m_pTaskCreationDialog->property("taskId").value<group_id>();

  ITask* pParentTask = m_pManager->task(taskId);
  TaskWidget* pParentTaskWidget = m_pWidgetManager->taskWidget(taskId);
  if (nullptr != pParentTask &&
      nullptr != pParentTaskWidget)
  {
    ITask* pTask = m_pManager->addTask();
    pTask->setName(m_pTaskCreationDialog->name());
    pTask->setDescription(m_pTaskCreationDialog->description());
    pParentTask->addTask(pTask->id());
    pParentTaskWidget->addTask(m_pWidgetManager->createTaskWidget(pTask->id()));

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
    TaskWidget* pTaskWidget = m_pWidgetManager->taskWidget(id);

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
    TaskWidget* pTaskWidget = m_pWidgetManager->taskWidget(id);

    ChangeTaskPropertyCommand* pChangeCommand =
        new ChangeTaskPropertyCommand(id, "description", pTask->description(), sNewDescr, m_pManager, m_pWidgetManager);
    m_undoStack.push(pChangeCommand);

    emit documentModified();
  }
}

void MainWindow::onTaskMoved(task_id id, group_id groupId, int iPos)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    TaskWidget* pTaskWidget = m_pWidgetManager->taskWidget(id);
    int iOldPos = pTask->priority().priority(0);
    IGroup* pOldGroup = m_pManager->group(pTask->group());
    GroupWidget* pOldGroupWidget = m_pWidgetManager->groupWidget(pTask->group());
    IGroup* pNewGroup = m_pManager->group(groupId);
    GroupWidget* pNewGroupWidget = m_pWidgetManager->groupWidget(groupId);

    // only 'move' the task if it really has been moved from one group to another
    if (nullptr != pOldGroup && (pOldGroup != pNewGroup || iOldPos != iPos))
    {
      MoveTaskCommand* pCommand = new MoveTaskCommand(id,
                                                      pTask->group(),
                                                      groupId,
                                                      iOldPos, iPos,
                                                      m_pManager,m_pWidgetManager);
      m_undoStack.push(pCommand);

      emit documentModified();
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
        QMessageBox::question(this, tr("Speichern"),
                              tr("Should the new document be opened?\n"
                                 "There are unsaved modifications.\n\n"
                                 "%1").arg(sLastSaveMessage));
    if (QMessageBox::No == button) { return; }
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
    pLabel->setObjectName("report");
    pLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    pOverlay->setAutoDeleteOnClose(true);
    pOverlay->addWidget(pLabel);
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

void MainWindow::onPropertyChanged(task_id taskId,
                                   const QString& sPropertyName,
                                   const QString& sValue)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask)
  {
    QString sOldValue = pTask->propertyValue(sPropertyName);
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
    pTask->removeProperty(sPropertyName);
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
        pTask->setPropertyValue("links", conversion::toString(links));
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
        pTask->setPropertyValue("links", conversion::toString(links));
      }
      else
      {
        // link not present - do nothing
        assert(false);
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
  ITask* pTask = m_pManager->task(parentTaskId);
  if (nullptr != pTask)
  {
    pTask->addTask(childTaskId);

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
  if (m_pManager->removeTask(id))
  {
    if (m_pWidgetManager->deleteTaskWidget(id))
    {
      emit documentModified();
    }
  }
}

void MainWindow::setAutoSortEnabled(group_id groupId)
{
  if (m_autoSortTimers.find(groupId) ==
      m_autoSortTimers.end())
  {
    m_autoSortTimers[groupId] = new QTimer(this);
    m_pTimeoutGroupIdMapper->setMapping(m_autoSortTimers[groupId], groupId);
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
