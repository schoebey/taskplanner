#include "widgetmanager.h"
#include "taskwidget.h"
#include "groupwidget.h"
#include "manager.h"
#include "taskinterface.h"
#include "groupinterface.h"
#include "conversion.h"
#include "task.h"
#include "group.h"

#include <QHBoxLayout>
#include <property.h>

WidgetManager::WidgetManager(Manager* pManager,
                               QWidget* pController)
  : m_pManager(pManager),
    m_pController(pController)
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

  QObject::connect(pGroupWidget, SIGNAL(renamed(group_id, QString)),          m_pController, SLOT(renameGroup(group_id, QString)));
  QObject::connect(pGroupWidget, SIGNAL(newTaskClicked(group_id)),            m_pController, SLOT(createNewTask(group_id)));
  QObject::connect(pGroupWidget, SIGNAL(taskMovedTo(task_id, group_id, int)), m_pController, SLOT(onTaskMoved(task_id, group_id, int)));
  QObject::connect(pGroupWidget, SIGNAL(autoSortEnabled(group_id)),           m_pController, SLOT(setAutoSortEnabled(group_id)));
  QObject::connect(pGroupWidget, SIGNAL(autoSortDisabled(group_id)),          m_pController, SLOT(setAutoSortDisabled(group_id)));

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
  TaskWidget* pTaskWidget = new TaskWidget(id);

  QObject::connect(pTaskWidget, SIGNAL(renamed(task_id, QString)),                  m_pController, SLOT(renameTask(task_id, QString)));
  QObject::connect(pTaskWidget, SIGNAL(descriptionChanged(task_id, QString)),       m_pController, SLOT(changeTaskDescription(task_id, QString)));
  QObject::connect(pTaskWidget, SIGNAL(timeTrackingStarted(task_id)),               m_pController, SLOT(startTimeTracking(task_id)));
  QObject::connect(pTaskWidget, SIGNAL(timeTrackingStopped(task_id)),               m_pController, SLOT(stopTimeTracking(task_id)));
  QObject::connect(pTaskWidget, SIGNAL(propertyChanged(task_id, QString, QString)), m_pController, SLOT(onPropertyChanged(task_id, QString, QString)));
  QObject::connect(pTaskWidget, SIGNAL(propertyRemoved(task_id, QString)),          m_pController, SLOT(onPropertyRemoved(task_id, QString)));
  QObject::connect(pTaskWidget, SIGNAL(taskMovedTo(task_id, task_id, int)),         m_pController, SLOT(onTaskMoved(task_id, task_id, int)));
  QObject::connect(pTaskWidget, SIGNAL(taskRemoved(task_id, task_id)),              m_pController, SLOT(onTaskRemoved(task_id, task_id)));
  QObject::connect(pTaskWidget, SIGNAL(taskDeleted(task_id)),                       m_pController, SLOT(onTaskDeleted(task_id)));
  QObject::connect(pTaskWidget, SIGNAL(newSubTaskRequested(task_id)),               m_pController, SLOT(createNewSubTask(task_id)));
  QObject::connect(pTaskWidget, SIGNAL(linkAdded(task_id, QUrl)),                   m_pController, SLOT(onLinkAdded(task_id, QUrl)));
  QObject::connect(pTaskWidget, SIGNAL(linkRemoved(task_id, QUrl)),                 m_pController, SLOT(onLinkRemoved(task_id, QUrl)));
  QObject::connect(pTaskWidget, SIGNAL(linkInserted(task_id, QUrl, int)),           m_pController, SLOT(onLinkInserted(task_id, QUrl, int)));
  QObject::connect(pTaskWidget, SIGNAL(priorityUpdateRequested(task_id)),           m_pController, SLOT(onPriorityUpdateRequested(task_id)));
  QObject::connect(m_pController, SIGNAL(timeTrackingStopped(task_id)),             pTaskWidget, SLOT(onTimeTrackingStopped(task_id)));


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
