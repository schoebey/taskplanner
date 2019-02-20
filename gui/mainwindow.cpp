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

#include <array>

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

  reloadStylesheet(":/stylesheet.css");

  QMetaObject::invokeMethod(this, "load", Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::load()
{
  size_t i = 0;
  std::array<QImage, 3> images = {{QImage(":/task_background_1.png"),
                                   QImage(":/task_background_2.png"),
                                   QImage(":/task_background_3.png")}};
  for (const auto& groupId : m_pManager->groupIds())
  {
    IGroup* pGroup = m_pManager->group(groupId);

    if (nullptr != pGroup)
    {
      GroupWidget* pGroupWidget = createGroupWidget(groupId);
      pGroupWidget->setBackgroundImage(images[i++ % images.size()]);
      pGroupWidget->setName(pGroup->name());

      bool bOk(false);
      bool bAutoSortingEnabled = conversion::fromString<bool>(pGroup->propertyValue("autoSorting"), bOk);
      if (bOk)  { pGroupWidget->setAutoSortingEnabled(bAutoSortingEnabled); }

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
          TaskWidget* pTaskWidget = createTaskWidget(pTask->id());

          auto pParentTask = m_pManager->task(pTask->parentTask());
          if (nullptr == pParentTask)
          {
            pGroupWidget->insertTask(pTaskWidget);
          }
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
}

void MainWindow::reloadStylesheet(const QString& sPath)
{
  qDebug() << sPath;
  QFile f(sPath);
  if (f.open(QIODevice::ReadOnly))
  {
    setStyleSheet(QString::fromUtf8(f.readAll()));
  }
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
  connect(pTaskWidget, SIGNAL(taskAdded(task_id, task_id)), this, SLOT(onTaskAdded(task_id, task_id)));
  connect(pTaskWidget, SIGNAL(taskRemoved(task_id, task_id)), this, SLOT(onTaskRemoved(task_id, task_id)));
  connect(pTaskWidget, SIGNAL(taskDeleted(task_id)), this, SLOT(onTaskDeleted(task_id)));
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
  }
}

void MainWindow::renameGroup(group_id id, const QString& sNewName)
{
  IGroup* pGroup = m_pManager->group(id);
  if (nullptr != pGroup)
  {
    pGroup->setName(sNewName);
  }
}

void MainWindow::renameTask(task_id id, const QString& sNewName)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    pTask->setName(sNewName);
  }
}

void MainWindow::changeTaskDescription(task_id id, const QString& sNewDescr)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    pTask->setDescription(sNewDescr);
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
}

void MainWindow::stopTimeTracking(task_id taskId)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask)
  {
    pTask->stopWork();
    emit timeTrackingStopped(taskId);
  }
}

void MainWindow::on_actionOpen_triggered()
{
  QString sFileName = QFileDialog::getOpenFileName(this, tr("Open task file..."));
  if (!sFileName.isEmpty())
  {

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
  if (!sFileName.isEmpty())
  {
    auto it = std::find_if(serializers.begin(), serializers.end(),
                           [sSelectedFilter](const std::pair<QString, QString>& p)
    { return QString("%1 (*.%2)").arg(p.first).arg(p.second) == sSelectedFilter; });
    if (it != serializers.end())
    {
      tspSerializer spWriter = SerializerFactory::create(it->first);
      if (!spWriter->setParameter("fileName", sFileName))
      {
        QMessageBox::critical(this, tr("error writing file"),
                              tr("parameter 'filename' not supported "
                                 "for serializer '%1'").arg(it->first));
      }

      ESerializingError err = m_pManager->serializeTo(spWriter.get());
      if (ESerializingError::eOk != err)
      {
        QString sErrorMessage;
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

        QMessageBox::critical(this, tr("error writing file"), sErrorMessage);
      }
    }
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
    }

    auto itTimer = m_autoSortTimers.find(pTask->group());
    if (itTimer != m_autoSortTimers.end() &&
        nullptr != itTimer->second &&
        itTimer->second->isActive())
    {
      sortGroup(pTask->group());
    }
  }
}

void MainWindow::onTaskAdded(task_id parentTaskId, task_id childTaskId)
{
  ITask* pTask = m_pManager->task(parentTaskId);
  if (nullptr != pTask)
  {
    pTask->addTask(childTaskId);
  }
}

void MainWindow::onTaskRemoved(task_id parentTaskId, task_id childTaskId)
{
  ITask* pTask = m_pManager->task(parentTaskId);
  if (nullptr != pTask)
  {
    pTask->removeTask(childTaskId);
  }
}

void MainWindow::onTaskDeleted(task_id id)
{
  if (m_pManager->removeTask(id))
  {
    auto it = m_taskWidgets.find(id);
    if (it != m_taskWidgets.end())
    {
      m_taskWidgets.erase(it);
      delete it->second;
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
