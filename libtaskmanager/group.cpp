#include "group.h"
#include "task.h"
#include "manager.h"
#include "serializerinterface.h"

Group::Group(Manager* pManager, group_id id)
  : IGroup(id),
    m_pManager(pManager)
{

}

Group::~Group()
{

}

int Group::version() const
{
  return 0;
}

ESerializingError Group::serialize(ISerializer* pSerializer) const
{
  return pSerializer->serialize(*this);
}

EDeserializingError Group::deserialize(ISerializer* pSerializer)
{
  return pSerializer->deserialize(*this);
}

QString Group::name() const
{
  return propertyValue("name");
}

void Group::setName(const QString& sName)
{
  setPropertyValue("name", sName);
}

QString Group::description() const
{
  return propertyValue("description");
}

void Group::setDescription(const QString& sDescription)
{
  setPropertyValue("description", sDescription);
}

std::set<task_id> Group::taskIds() const
{
  return m_tasksIds;
}

bool Group::addTaskId(task_id taskId)
{
  return m_tasksIds.insert(taskId).second;
}

bool Group::addTask(task_id taskId)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask && addTaskId(taskId))
  {
    pTask->setGroup(id());
    SPriority prio = pTask->priority();
    prio.setPriority(0, static_cast<int>(m_tasksIds.size() - 1));
    pTask->setPriority(prio);
    return true;
  }

  return false;
}

bool Group::removeTask(task_id taskId)
{
  auto it = std::find(m_tasksIds.begin(), m_tasksIds.end(), taskId);
  if (it != m_tasksIds.end())
  {
    m_tasksIds.erase(it);

    ITask* pTask = m_pManager->task(taskId);
    if (nullptr != pTask)
    {
      pTask->setGroup(-1);
    }

    return true;
  }

  return false;
}

std::set<QString> Group::propertyNames() const
{
  return m_properties.availablePropertyNames();
}

bool Group::hasPropertyValue(const QString& sName) const
{
  return m_properties.isValid(sName);
}

QString Group::propertyValue(const QString& sName) const
{
  return m_properties.get(sName);
}

bool Group::setPropertyValue(const QString& sName, const QString& sValue)
{
  return m_properties.set(sName, sValue);
}

bool Group::removeProperty(const QString& sName)
{
  return m_properties.remove(sName);
}
