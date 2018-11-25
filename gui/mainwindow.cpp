#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "groupwidget.h"
#include "taskwidget.h"
#include "manager.h"
#include "groupinterface.h"
#include "taskinterface.h"


#include <QFileSystemWatcher>
#include <QDebug>
#include <QFile>

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
  int i = 0;
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
      for (const auto& taskId : pGroup->taskIds())
      {
        ITask* pTask = m_pManager->task(taskId);
        if (nullptr != pTask)
        {
          TaskWidget* pTaskWidget = createTaskWidget(taskId);
          pTaskWidget->setName(pTask->name());
          pTaskWidget->setDescription(pTask->description());
          pGroupWidget->InsertTask(pTaskWidget);
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

void MainWindow::moveTask(task_id id, group_id groupId, int /*iPos*/)
{
  ITask* pTask = m_pManager->task(id);
  if (nullptr != pTask)
  {
    pTask->setGroup(groupId);

    // recalculate the priorities based on
    // the current sort order and the new position of the task
  }
}

void MainWindow::startTimeTracking(task_id taskId)
{
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
  }
}
