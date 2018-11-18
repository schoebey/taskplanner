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
  for (const auto& groupId : m_pManager->groupIds())
  {

    IGroup* pGroup = m_pManager->group(groupId);

    if (nullptr != pGroup)
    {
      GroupWidget* pGroupWidget = createGroupWidget(groupId);
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
  // task_id taskId = manager.addTask(groupId);
  // emit taskAdded(taskId, groupId);

  task_id newId = 0;

  auto it = m_groupWidgets.find(groupId);
  if (it != m_groupWidgets.end())
  {
    it->second->InsertTask(createTaskWidget(newId));
  }
}

void MainWindow::renameGroup(group_id id, const QString& sNewName)
{
  qDebug() << "group" << id << "was renamed to " << sNewName;
}

void MainWindow::renameTask(task_id id, const QString& sNewName)
{
  qDebug() << "task" << id << "was renamed to " << sNewName;
}

void MainWindow::changeTaskDescription(task_id id, const QString& sNewDescr)
{
  qDebug() << "task" << id << "has a new description:" << sNewDescr;
}

void MainWindow::moveTask(task_id id, group_id groupId, int iPos)
{

  qDebug() << "task" << id << "was moved to group" << groupId << "at position" << iPos;
}
