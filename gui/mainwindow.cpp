#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "groupwidget.h"
#include "taskwidget.h"
#include "manager.h"
#include "groupinterface.h"
#include "taskinterface.h"
#include "serializerfactory.h"
#include "reportfactory.h"
#include "property.h"
#include "overlaywidget.h"
#include "taskcreationdialog.h"


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
  m_pTimeoutGroupIdMapper(new QSignalMapper(this))
{
  ui->setupUi(this);

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
  for (const auto& el : m_taskWidgets)
  {
    delete el.second;
  }
  m_taskWidgets.clear();

  for (const auto& el : m_groupWidgets)
  {
    delete el.second;
  }
  m_groupWidgets.clear();

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
      GroupWidget* pGroupWidget = nullptr;
      auto itGroupWidget = m_groupWidgets.find(groupId);
      if (m_groupWidgets.end() == itGroupWidget)
      {
        pGroupWidget = createGroupWidget(groupId);
        pGroupWidget->setName(pGroup->name());
      }
      else
      {
        pGroupWidget = itGroupWidget->second;
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
          TaskWidget* pTaskWidget = nullptr;
          auto itTaskWidget = m_taskWidgets.find(pTask->id());
          if (m_taskWidgets.end() == itTaskWidget)
          {
            pTaskWidget = createTaskWidget(pTask->id());

            auto pParentTask = m_pManager->task(pTask->parentTask());
            if (nullptr == pParentTask)
            {
              pGroupWidget->insertTask(pTaskWidget);
            }
          }
        }
      }


      // afer all the tasks have been created, the group properties can be set
      for (auto el : m_groupWidgets)
      {
        pGroupWidget = el.second;
        IGroup* pGroup = m_pManager->group(el.first);

        if (nullptr != pGroup)
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
      auto it = m_taskWidgets.find(taskId);
      if (it != m_taskWidgets.end() &&
          nullptr != it->second)
      {
        auto parentIt = m_taskWidgets.find(pTask->parentTask());
        if (parentIt != m_taskWidgets.end() &&
            nullptr != parentIt->second)
        {
          parentIt->second->addTask(it->second);
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

GroupWidget* MainWindow::createGroupWidget(group_id id)
{
  QHBoxLayout* pLayout = dynamic_cast<QHBoxLayout*>(ui->frame->layout());

  GroupWidget* pGroupWidget = new GroupWidget(id);
  pLayout->addWidget(pGroupWidget);

  connect(pGroupWidget, SIGNAL(renamed(group_id, QString)), this, SLOT(renameGroup(group_id, QString)));
  connect(pGroupWidget, SIGNAL(newTaskClicked(group_id)), this, SLOT(createNewTask(group_id)));
  connect(pGroupWidget, SIGNAL(taskMovedTo(task_id, group_id, int)), this, SLOT(moveTask(task_id, group_id, int)));
  connect(pGroupWidget, SIGNAL(autoSortEnabled(group_id)), this, SLOT(setAutoSortEnabled(group_id)));
  connect(pGroupWidget, SIGNAL(autoSortDisabled(group_id)), this, SLOT(setAutoSortDisabled(group_id)));

  m_groupWidgets[id] = pGroupWidget;

  return pGroupWidget;
}

TaskWidget* MainWindow::createTaskWidget(task_id id)
{
  TaskWidget* pTaskWidget = new TaskWidget(id);

  connect(pTaskWidget, SIGNAL(renamed(task_id, QString)), this, SLOT(renameTask(task_id, QString)));
  connect(pTaskWidget, SIGNAL(descriptionChanged(task_id, QString)), this, SLOT(changeTaskDescription(task_id, QString)));
  connect(pTaskWidget, SIGNAL(timeTrackingStarted(task_id)), this, SLOT(startTimeTracking(task_id)));
  connect(pTaskWidget, SIGNAL(timeTrackingStopped(task_id)), this, SLOT(stopTimeTracking(task_id)));
  connect(pTaskWidget, SIGNAL(propertyChanged(task_id, QString, QString)), this, SLOT(onPropertyChanged(task_id, QString, QString)));
  connect(pTaskWidget, SIGNAL(propertyRemoved(task_id, QString)), this, SLOT(onPropertyRemoved(task_id, QString)));
  connect(pTaskWidget, SIGNAL(taskAdded(task_id, task_id)), this, SLOT(onTaskAdded(task_id, task_id)));
  connect(pTaskWidget, SIGNAL(taskRemoved(task_id, task_id)), this, SLOT(onTaskRemoved(task_id, task_id)));
  connect(pTaskWidget, SIGNAL(taskDeleted(task_id)), this, SLOT(onTaskDeleted(task_id)));
  connect(pTaskWidget, SIGNAL(newSubTaskRequested(task_id)), this, SLOT(createNewSubTask(task_id)));
  connect(pTaskWidget, SIGNAL(linkAdded(task_id, QUrl)), this, SLOT(onLinkAdded(task_id, QUrl)));
  connect(pTaskWidget, SIGNAL(linkRemoved(task_id, QUrl)), this, SLOT(onLinkRemoved(task_id, QUrl)));
  connect(pTaskWidget, SIGNAL(linkInserted(task_id, QUrl, int)), this, SLOT(onLinkInserted(task_id, QUrl, int)));
  connect(this, SIGNAL(timeTrackingStopped(task_id)), pTaskWidget, SLOT(onTimeTrackingStopped(task_id)));


  auto pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    pTaskWidget->setName(pTask->name());
    pTaskWidget->setDescription(pTask->description());
  }

  bool bOk = false;
  bool bExpanded = conversion::fromString<bool>(pTask->propertyValue("expanded"), bOk);

  // konnte die Property ausgelesen werden, soll der expanded-State wiederhergestellt werden,
  // sonst soll defaultmässig expandiert sein.
  pTaskWidget->setExpanded(!bOk || bExpanded);


  // TODO: hier die Links auslesen und via pTaskWidget->addLink() einzeln hinzufügen
  // TODO: im Taskwidget: addLink, removeLink, insertLink(pos, link)
  auto links = conversion::fromString<std::vector<QUrl>>(pTask->propertyValue("links"), bOk);
  if (bOk)
  {
    for (const auto& link : links)
    {
      pTaskWidget->addLink(link);
    }
  }

  auto color = conversion::fromString<QColor>(pTask->propertyValue("color"), bOk);
  if (bOk)
  {
    pTaskWidget->setOverlayBackground(color);
  }

  for (const QString& sName : Properties<Task>::registeredPropertyNames())
  {
    if (!Properties<Task>::visible(sName))  { continue; }

    if (pTask->hasPropertyValue(sName))
    {
      QString sPropertyValue = pTask->propertyValue(sName);
      pTaskWidget->addProperty(sName, sPropertyValue);
    }
  }

  m_taskWidgets[id] = pTaskWidget;

  return pTaskWidget;
}

void MainWindow::createNewTask(group_id groupId)
{
  IGroup* pGroup = m_pManager->group(groupId);
  if (nullptr != pGroup)
  {
    auto it = m_groupWidgets.find(groupId);
    if (it != m_groupWidgets.end())
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
  auto it = m_groupWidgets.find(groupId);
  if (nullptr != pGroup &&
      it != m_groupWidgets.end())
  {
    ITask* pTask = m_pManager->addTask();
    pTask->setName(m_pTaskCreationDialog->name());
    pTask->setDescription(m_pTaskCreationDialog->description());
    pGroup->addTask(pTask->id());
    it->second->insertTask(createTaskWidget(pTask->id()));

    emit documentModified();
  }
}

void MainWindow::createNewSubTask(task_id taskId)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask)
  {
    auto it = m_taskWidgets.find(taskId);
    if (it != m_taskWidgets.end())
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
  auto it = m_taskWidgets.find(taskId);
  if (nullptr != pParentTask &&
      it != m_taskWidgets.end())
  {
    ITask* pTask = m_pManager->addTask();
    pTask->setName(m_pTaskCreationDialog->name());
    pTask->setDescription(m_pTaskCreationDialog->description());
    pParentTask->addTask(pTask->id());
    it->second->addTask(createTaskWidget(pTask->id()));

    emit documentModified();
  }
}

void MainWindow::renameGroup(group_id id, const QString& sNewName)
{
  IGroup* pGroup = m_pManager->group(id);
  if (nullptr != pGroup)
  {
    pGroup->setName(sNewName);

    emit documentModified();
  }
}

void MainWindow::renameTask(task_id id, const QString& sNewName)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    pTask->setName(sNewName);

    emit documentModified();
  }
}

void MainWindow::changeTaskDescription(task_id id, const QString& sNewDescr)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    pTask->setDescription(sNewDescr);

    emit documentModified();
  }
}

void MainWindow::moveTask(task_id id, group_id groupId, int iPos)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    group_id oldGroupId = pTask->group();

    pTask->setGroup(groupId);


    // if the task has moved groups, fill the priority gaps in the old group
    // by building a sequence, determining the jumps and correcting the priorities
    // of following tasks.
    if (oldGroupId != groupId)
    {
      IGroup* pOldGroup = m_pManager->group(oldGroupId);
      if (nullptr != pOldGroup)
      {
        std::map<int, ITask*> tasksByPriority;
        for (const auto& taskId : pOldGroup->taskIds())
        {
          ITask* pOtherTask = m_pManager->task(taskId);
          if (nullptr != pOtherTask)
          {
            tasksByPriority[pOtherTask->priority().priority(0)] = pOtherTask;
          }
        }

        // determine the gaps
        int iPrevPrio = -1;
        int iDelta = 0;
        for (const auto& el : tasksByPriority)
        {
          SPriority prio = el.second->priority();
          iDelta += el.first - iPrevPrio - 1;
          iPrevPrio = prio.priority(0);
          if (0 < iDelta)
          {
            prio.setPriority(0, iPrevPrio - iDelta);
            el.second->setPriority(prio);
          }
        }
      }
    }



    SPriority prio = pTask->priority();

    IGroup* pGroup = m_pManager->group(groupId);
    if (nullptr != pGroup)
    {
      // increment the priority of every task that is below the moved task, by one.
      for (const auto& taskId : pGroup->taskIds())
      {
        ITask* pOtherTask = m_pManager->task(taskId);
        if (nullptr != pOtherTask)
        {
          // if the item's priority lies between the old and the new priority, increment it by one
          SPriority otherPrio = pOtherTask->priority();
          if (iPos <= otherPrio.priority(0)  &&
              prio.priority(0) >= otherPrio.priority(0))
          {
            otherPrio.setPriority(0, otherPrio.priority(0) + 1);
          }
          else if (iPos >= otherPrio.priority(0)  &&
              prio.priority(0) < otherPrio.priority(0))
          {
            otherPrio.setPriority(0, otherPrio.priority(0) - 1);
          }

          pOtherTask->setPriority(otherPrio);
        }
      }
    }


    prio.setPriority(0, iPos);
    pTask->setPriority(prio);

    emit documentModified();
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
    bool bNewValueAccepted = pTask->setPropertyValue(sPropertyName, sValue);


    auto it = m_taskWidgets.find(taskId);
    if (it != m_taskWidgets.end())
    {
      it->second->setHighlight(it->second->highlight() |
                               (bNewValueAccepted ? EHighlightMethod::eValueAccepted :
                                                   EHighlightMethod::eValueRejected));
      it->second->setPropertyValue(sPropertyName, pTask->propertyValue(sPropertyName));

      if ("color" == sPropertyName)
      {
        bool bOk(false);
        auto color = conversion::fromString<QColor>(pTask->propertyValue("color"), bOk);
        if (bOk)
        {
          it->second->setOverlayBackground(color);
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
    pTask->setPropertyValue(sPropertyName, QString());
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
    auto it = m_taskWidgets.find(id);
    if (it != m_taskWidgets.end())
    {
      delete it->second;
      m_taskWidgets.erase(it);

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


  auto it = m_groupWidgets.find(groupId);
  if (it != m_groupWidgets.end())
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
    it->second->reorderTasks(vIds);
  }
}

void MainWindow::sortGroups()
{
  for (const auto& el : m_groupWidgets)
  {
    if (nullptr != el.second)
    {
      sortGroup(el.second->id());
    }
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
      auto it = m_taskWidgets.find(taskId);
      if (it != m_taskWidgets.end() &&
          nullptr != it->second)
      {
        TaskWidget* pWidget = it->second;
        pWidget->setAutoPriority(pTask->autoPriority());
      }
    }
  }
}

void MainWindow::timerEvent(QTimerEvent* /*pEvent*/)
{
  updateAutoPrioritiesInTaskWidgets();
}
