#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "groupwidget.h"
#include "taskwidget.h"
#include "manager.h"
#include "groupinterface.h"
#include "taskinterface.h"
#include "serializerfactory.h"
#include "property.h"


#include <QFileSystemWatcher>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include <array>

MainWindow::MainWindow(Manager* pManager, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_pManager(pManager)
{
  ui->setupUi(this);


  QFileSystemWatcher* pWatcher = new QFileSystemWatcher(this);
  pWatcher->addPath("stylesheet.css");
  connect(pWatcher, SIGNAL(fileChanged(QString)), this, SLOT(reloadStylesheet(QString)));

  reloadStylesheet("stylesheet.css");

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
          pTaskWidget->setName(pTask->name());
          pTaskWidget->setDescription(pTask->description());

          bool bOk = false;
          bool bExpanded = conversion::fromString<bool>(pTask->propertyValue("expanded"), bOk);

          // konnte die Property ausgelesen werden, soll der expanded-State wiederhergestellt werden,
          // sonst soll defaultmässig expandiert sein.
          pTaskWidget->setExpanded(!bOk || bExpanded);
          for (const QString& sName : Properties::registeredPropertyNames())
          {
            if (!Properties::visible(sName))  { continue; }

            QString sPropertyValue = pTask->propertyValue(sName);
            //if (!sPropertyValue.isEmpty())
            {
              pTaskWidget->addProperty(sName, sPropertyValue);
            }
          }

          pGroupWidget->InsertTask(pTaskWidget, task.first);
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
  connect(this, SIGNAL(timeTrackingStopped(task_id)), pTaskWidget, SLOT(onTimeTrackingStopped(task_id)));

  m_taskWidgets[id] = pTaskWidget;

  return pTaskWidget;
}

void MainWindow::createNewTask(group_id groupId)
{
  IGroup* pGroup = m_pManager->group(groupId);
  if (nullptr != pGroup)
  {
   ITask* pTask = m_pManager->addTask();
   pGroup->addTask(pTask->id());

   auto it = m_groupWidgets.find(groupId);
   if (it != m_groupWidgets.end())
   {
     it->second->InsertTask(createTaskWidget(pTask->id()));
   }
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
  auto serializers = SerializerFactory::availableSerializers();
  for (const auto& name : serializers)
  {
    list.append(QString("%1 (*.%2)").arg(name.sName).arg(name.sFileExtension));
  }

  QString sFilter = list.join(";;");
  QString sSelectedFilter;
  QString sFileName = QFileDialog::getSaveFileName(this, tr("Save task file as..."),
                                                   QString(), sFilter,
                                                   &sSelectedFilter);
  if (!sFileName.isEmpty())
  {
    size_t idx = static_cast<size_t>(list.indexOf(sSelectedFilter));
    if (idx < serializers.size())
    {
      tspSerializer spWriter = SerializerFactory::create(serializers[idx].sName);
      if (!spWriter->setParameter("fileName", sFileName))
      {
        QMessageBox::critical(this, tr("error writing file"),
                              tr("parameter 'filename' not supported "
                                 "for serializer '%1'").arg(serializers[idx].sName));
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

void MainWindow::on_actionReport_triggered()
{
  QString sFileName = QFileDialog::getSaveFileName(this, tr("save report as..."));
  if (!sFileName.isEmpty())
  {

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
  }
}
