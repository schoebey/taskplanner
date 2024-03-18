#include "widgetmanager.h"
#include "taskwidget.h"
#include "groupwidget.h"
#include "manager.h"
#include "taskinterface.h"
#include "groupinterface.h"
#include "conversion.h"
#include "task.h"
#include "group.h"
#include "taskcontrollerinterface.h"
#include "groupcontrollerinterface.h"

#include <QHBoxLayout>
#include <property.h>

WidgetManager::WidgetManager(Manager* pManager,
                             ITaskController* pTaskController,
                             IGroupController* pGroupController,
                             QWidget* pParentWidget)
  : m_pManager(pManager),
    m_pTaskController(pTaskController),
    m_pGroupController(pGroupController),
    m_pParentWidget(pParentWidget)
{

}

void WidgetManager::clear()
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
}

TaskWidget* WidgetManager::taskWidget(task_id id) const
{
  auto it = m_taskWidgets.find(id);
  if (it != m_taskWidgets.end())
  {
    return it->second;
  }

  return nullptr;
}

GroupWidget* WidgetManager::groupWidget(group_id id) const
{
  auto it = m_groupWidgets.find(id);
  if (it != m_groupWidgets.end())
  {
    return it->second;
  }

  return nullptr;
}

GroupWidget* WidgetManager::createGroupWidget(group_id id)
{
  GroupWidget* pGroupWidget = new GroupWidget(id);

  QObject::connect(pGroupWidget, &GroupWidget::renamed,             std::bind(&IGroupController::renameGroup, m_pGroupController, std::placeholders::_1, std::placeholders::_2));
  QObject::connect(pGroupWidget, &GroupWidget::newTaskClicked,      std::bind(&IGroupController::createNewTask, m_pGroupController, std::placeholders::_1));
  QObject::connect(pGroupWidget, &GroupWidget::taskMovedTo,         std::bind(&IGroupController::onTaskMoved, m_pGroupController, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  QObject::connect(pGroupWidget, &GroupWidget::autoSortEnabled,     std::bind(&IGroupController::setAutoSortEnabled, m_pGroupController, std::placeholders::_1));
  QObject::connect(pGroupWidget, &GroupWidget::autoSortDisabled,    std::bind(&IGroupController::setAutoSortDisabled, m_pGroupController, std::placeholders::_1));

  IGroup* pGroup = m_pManager->group(id);
  if (nullptr != pGroup)
  {
    pGroupWidget->setName(pGroup->name());
  }

  m_groupWidgets[id] = pGroupWidget;

  return pGroupWidget;
}

TaskWidget* WidgetManager::createTaskWidget(task_id id)
{

  TaskWidget* pTaskWidget = new TaskWidget(id, m_pParentWidget);

  QObject::connect(pTaskWidget, &TaskWidget::renamed,                      std::bind(&ITaskController::renameTask, m_pTaskController, std::placeholders::_1, std::placeholders::_2));
  QObject::connect(pTaskWidget, &TaskWidget::descriptionChanged,           std::bind(&ITaskController::changeTaskDescription, m_pTaskController, std::placeholders::_1, std::placeholders::_2));
  QObject::connect(pTaskWidget, &TaskWidget::timeTrackingStarted,          std::bind(&ITaskController::startTimeTracking, m_pTaskController, std::placeholders::_1));
  QObject::connect(pTaskWidget, &TaskWidget::timeTrackingStopped,          std::bind(&ITaskController::stopTimeTracking, m_pTaskController, std::placeholders::_1));
  QObject::connect(pTaskWidget, &TaskWidget::propertyChanged,              std::bind(&ITaskController::onPropertyChanged, m_pTaskController, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  QObject::connect(pTaskWidget, &TaskWidget::propertyRemoved,              std::bind(&ITaskController::onPropertyRemoved, m_pTaskController, std::placeholders::_1, std::placeholders::_2));
  QObject::connect(pTaskWidget, &TaskWidget::taskMovedTo,                  std::bind(&ITaskController::onTaskMoved, m_pTaskController, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  QObject::connect(pTaskWidget, &TaskWidget::taskRemoved,                  std::bind(&ITaskController::onTaskRemoved, m_pTaskController, std::placeholders::_1, std::placeholders::_2));
  QObject::connect(pTaskWidget, &TaskWidget::taskDeleted,                  std::bind(&ITaskController::onTaskDeleted, m_pTaskController, std::placeholders::_1));
  QObject::connect(pTaskWidget, &TaskWidget::newSubTaskRequested,          std::bind(&ITaskController::createNewSubTask, m_pTaskController, std::placeholders::_1));
  QObject::connect(pTaskWidget, &TaskWidget::addTimeRequested,             std::bind(&ITaskController::onAddTimeToTaskRequested, m_pTaskController, std::placeholders::_1));
  QObject::connect(pTaskWidget, &TaskWidget::removeTimeRequested,          std::bind(&ITaskController::onRemoveTimeFromTaskRequested, m_pTaskController, std::placeholders::_1));
  QObject::connect(pTaskWidget, &TaskWidget::linkAdded,                    std::bind(&ITaskController::onLinkAdded, m_pTaskController, std::placeholders::_1, std::placeholders::_2));
  QObject::connect(pTaskWidget, &TaskWidget::linkRemoved,                  std::bind(&ITaskController::onLinkRemoved, m_pTaskController, std::placeholders::_1, std::placeholders::_2));
  QObject::connect(pTaskWidget, &TaskWidget::linkInserted,                 std::bind(&ITaskController::onLinkInserted, m_pTaskController, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  QObject::connect(pTaskWidget, &TaskWidget::priorityUpdateRequested,      std::bind(&ITaskController::onPriorityUpdateRequested, m_pTaskController, std::placeholders::_1));
  QObject::connect(pTaskWidget, &TaskWidget::priorityUpdateRequested,      std::bind(&ITaskController::onTimeTrackingStopped, m_pTaskController, std::placeholders::_1));
  QObject::connect(pTaskWidget, &TaskWidget::childPropertyChangeRequested, std::bind(&ITaskController::onChildPropertyChangeRequested, m_pTaskController, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  QObject::connect(pTaskWidget, &TaskWidget::autoPriorityUpdateRequested,  std::bind(&ITaskController::onAutoPriorityUpdateRequested, m_pTaskController, std::placeholders::_1));

  QObject::connect(m_pParentWidget, SIGNAL(timeTrackingStopped(task_id)),             pTaskWidget, SLOT(onTimeTrackingStopped(task_id)));


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

  // let the widget know which priority it has to visualize
  pTaskWidget->setAutoPriority(pTask->autoPriority());

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

bool WidgetManager::deleteTaskWidget(task_id id)
{
  auto it = m_taskWidgets.find(id);
  if (it != m_taskWidgets.end())
  {
    delete it->second;
    m_taskWidgets.erase(it);
    return true;
  }

  return false;
}

bool WidgetManager::deleteGroupWidget(group_id id)
{
  auto it = m_groupWidgets.find(id);
  if (it != m_groupWidgets.end())
  {
    delete it->second;
    m_groupWidgets.erase(it);
    return true;
  }

  return false;
}
