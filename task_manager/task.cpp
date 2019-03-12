#include "task.h"
#include "group.h"
#include "manager.h"
#include "serializerinterface.h"

Task::Task(Manager* pManager, task_id id)
 : ITask(id),
   m_pManager(pManager)
{

}

Task::~Task()
{

}

int Task::version() const
{
  return 0;
}

ESerializingError Task::serialize(ISerializer* pSerializer) const
{
  return pSerializer->serialize(*this);
}

EDeserializingError Task::deserialize(ISerializer* pSerializer)
{
  return pSerializer->deserialize(*this);
}

QString Task::name() const
{
  return propertyValue("name");
}

void Task::setName(const QString& sName)
{
  setPropertyValue("name", sName);
}

QString Task::description() const
{
  return propertyValue("description");
}

void Task::setDescription(const QString& sDescription)
{
  setPropertyValue("description", sDescription);
}

SPriority Task::priority() const
{
  return m_priority;
}

void Task::setPriority(const SPriority& priority)
{
  m_priority = priority;
}

double Task::autoPriority() const
{
  // compute a priority value for each available property
  // assume 0 for any unavailable property
  // for now: sum them up and return the result


  // ideally, constraints could be used to compute a priority value.
  // the problem is that constraints can theoretically be rather complex constructs
  // that make it hard to compute such a value.
  // an and constraint would need to query both sub-branches and somehow interpolate between them
  // e.g.  min(5) and max(10) would need to interpolate between the priority values of 5 and 10
  // (min(5) and max(10)) or (min(12) and max(19)) would need to do so for the sub-branch that accepts the given value.
  // meaning: and-constaints would need to query left and right for (min,max) values and assigned priorities
  // or-constraints would need to delegate the call to the left or right sub-branch, depending on which one accepts the value.


  // for now: use due date, added date and duration as a base for computing the priority
  double dDueTimeWeight = 0;
  int iNofDaysTaskShouldBeInProgress = 0;
  QDateTime dueDate = property<QDateTime>("due date");
  if (dueDate.isValid())
  {
    qint64 iTimeToDue = std::max<qint64>(1, QDateTime::currentDateTime().secsTo(dueDate));

    // piecewise linear interpolation based on the segment we're in
    std::vector<qint64> viPivots = {18000,
                                    24 * 3600,
                                    2 * 24 * 3600,
                                    7 * 24 * 3600,
                                    14 * 24 * 3600,
                                    30 * 24 * 3600,
                                    2 * 30 * 24 * 3600,
                                    6 * 30 * 24 * 3600,
                                    3600,
                                    600,
                                    0};
    std::sort(viPivots.begin(), viPivots.end(), std::greater<qint64>());
    dDueTimeWeight = viPivots.size() - 1;
    for (size_t idx = viPivots.size() - 2; idx >= 0; --idx)
    {
      if (iTimeToDue < viPivots[idx])
      {
        double dFractionInSegment =
            static_cast<double>(viPivots[idx] - iTimeToDue) /
            (viPivots[idx] - viPivots[idx + 1]);

        dDueTimeWeight = dFractionInSegment + idx;

        break;
      }
    }

//    // e.g. based on the actual time to due, not just discreet buckets.
//    // set prio category according to the time left
//    if (iTimeToDue < 600) // less than 10 mins
//    {
//      dDueTimeWeight = 10;
//    }
//    else if (iTimeToDue < 3600) // less than an hour
//    {
//      dDueTimeWeight = 9;
//    }
//    else if (iTimeToDue < 18000) // less than half a day
//    {
//      dDueTimeWeight = 8;
//    }
//    else if (iTimeToDue < 24 * 3600) // less than a day
//    {
//      dDueTimeWeight = 7;
//    }
//    else if (iTimeToDue < 2 * 24 * 3600) // less than two days
//    {
//      dDueTimeWeight = 6;
//    }
//    else if (iTimeToDue < 7 * 24 * 3600) // less than a week
//    {
//      dDueTimeWeight = 5;
//    }
//    else if (iTimeToDue < 14 * 3600) // less than two weeks
//    {
//      dDueTimeWeight = 4;
//    }
//    else if (iTimeToDue < 30 * 3600) // less than a month
//    {
//      dDueTimeWeight = 3;
//    }
//    else if (iTimeToDue < 60 * 3600) // less than two months
//    {
//      dDueTimeWeight = 2;
//    }
//    else if (iTimeToDue < 182 * 24 * 3600) // less than half a year
//    {
//      dDueTimeWeight = 1;
//    }
//    else // everything longer than half a year
//    {
//      dDueTimeWeight = 0;
//    }


    int iNofDays = property<int>("duration (days)");
    QDateTime startDate = dueDate.addDays(-iNofDays);

    int iNofDaysToStart = QDateTime::currentDateTime().daysTo(startDate);
    if (0 >= iNofDaysToStart)
    {
      int iNofDaysToDue = QDateTime::currentDateTime().daysTo(dueDate);
      iNofDaysTaskShouldBeInProgress = iNofDays - iNofDaysToDue;

//      dDueTimeWeight += something;
    }
  }


  int iNofDaysInSystem = 0;
  QDateTime addedDate = property<QDateTime>("added date");
  if (addedDate.isValid())
  {
    iNofDaysInSystem = addedDate.daysTo(QDateTime::currentDateTime());
  }


  // TODO: incorporate factor (hi/med/lo)
  // TODO: replace with std::tie
  return dDueTimeWeight +
      static_cast<double>(iNofDaysInSystem) / 100.;
}

task_id Task::parentTask() const
{
  return m_parentTaskId;
}

void Task::setParentTaskId(task_id parentTaskId)
{
  m_parentTaskId = parentTaskId;
}

void Task::setParentTask(task_id parentTaskId)
{
  if (m_parentTaskId != parentTaskId)
  {
    ITask* pParent = m_pManager->task(m_parentTaskId);
    if (nullptr != pParent)
    {
      pParent->removeTask(id());
    }

    setParentTaskId(parentTaskId);
    pParent = m_pManager->task(parentTaskId);
    if (nullptr != pParent)
    {
      pParent->addTask(id());
    }
  }
}

std::set<task_id> Task::taskIds() const
{
  return m_subTaskIds;
}

bool Task::addTask(task_id taskId)
{
  ITask* pTask = m_pManager->task(taskId);
  if (nullptr != pTask && addTaskId(taskId))
  {
    pTask->setParentTask(id());
    return true;
  }

  return false;
}

bool Task::addTaskId(task_id id)
{
  if (id == this->id())  { return false; }

  return m_subTaskIds.insert(id).second;
}

bool Task::removeTask(task_id id)
{
  auto it = m_subTaskIds.find(id);
  if (it != m_subTaskIds.end())
  {
    m_subTaskIds.erase(it);

    ITask* pTask = m_pManager->task(id);
    if (nullptr != pTask)
    {
      pTask->setParentTask(-1);
    }
    return true;
  }

  return false;
}

void Task::startWork()
{
  STimeFragment fragment;
  fragment.startTime = QDateTime::currentDateTime();
  m_vTimingInfo.push_back(fragment);
}

void Task::stopWork()
{
  if (m_vTimingInfo.empty())  { return; }

  if (!m_vTimingInfo.back().stopTime.isValid())
  {
    m_vTimingInfo.back().stopTime = QDateTime::currentDateTime();
  }
}

std::vector<STimeFragment> Task::timeFragments() const
{
  return m_vTimingInfo;
}

void Task::setTimeFragments(const std::vector<STimeFragment>& vFragments)
{
  m_vTimingInfo = vFragments;
}

group_id Task::group() const
{
  return m_groupId;
}

void Task::setGroup(group_id groupId)
{
  if (m_groupId != groupId)
  {
    IGroup* pGroup = m_pManager->group(m_groupId);
    if (nullptr != pGroup)
    {
      pGroup->removeTask(id());
    }

    m_groupId = groupId;

    pGroup = m_pManager->group(m_groupId);
    if (nullptr != pGroup)
    {
      pGroup->addTask(id());
    }
  }

  for (const auto& subTaskId : m_subTaskIds)
  {
    ITask* pSubTask = m_pManager->task(subTaskId);
    if (nullptr != pSubTask)  { pSubTask->setGroup(groupId); }
  }
}

std::set<QString> Task::propertyNames() const
{
  return m_properties.availablePropertyNames();
}

bool Task::hasPropertyValue(const QString& sName) const
{
  return m_properties.isValid(sName);
}

QString Task::propertyValue(const QString& sName) const
{
  return m_properties.get(sName);
}

bool Task::setPropertyValue(const QString& sName, const QString& sValue)
{
  return m_properties.set(sName, sValue);
}
