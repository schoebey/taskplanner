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
  return 2;
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
  setProperty<int>("sort_priority", m_priority.categories[0]);
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
    for (size_t idx = viPivots.size() - 2;; --idx)
    {
      if (iTimeToDue < viPivots[idx])
      {
        double dFractionInSegment =
            static_cast<double>(viPivots[idx] - iTimeToDue) /
            (viPivots[idx] - viPivots[idx + 1]);

        dDueTimeWeight = dFractionInSegment + idx;

        break;
      }

      if (0 == idx) { break; }
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


//    int iNofDays = property<int>("duration (days)");
//    QDateTime startDate = dueDate.addDays(-iNofDays);
//   
//    int iNofDaysToStart = QDateTime::currentDateTime().daysTo(startDate);
//    if (0 >= iNofDaysToStart)
//    {
//      int iNofDaysToDue = QDateTime::currentDateTime().daysTo(dueDate);
//        int iNofDaysTaskShouldBeInProgress = iNofDays - iNofDaysToDue;
//   
//        dDueTimeWeight += something;
//    }
  }


  int iNofDaysInSystem = 0;
  QDateTime addedDate = property<QDateTime>("added date");
  if (addedDate.isValid())
  {
    iNofDaysInSystem = addedDate.daysTo(QDateTime::currentDateTime());
  }


  // TODO: incorporate factor (hi/med/lo)
  // TODO: replace with std::tie
  int iUserDefinedPriority = 0;
  if (hasPropertyValue("priority"))
  {
    iUserDefinedPriority = property<int>("priority");
  }

  // nof days in the system should only modify the auto priority in the range [0, 1[
  // therefore, the max time in system is 10 years. All tasks older than that will get
  // the same modifier and ultimately the same autoPriority.
  double dAutoPriority = std::max<double>(dDueTimeWeight, iUserDefinedPriority) +
      std::min<double>(0.999999, static_cast<double>(iNofDaysInSystem) / 3650.);


  // determine the maximum of all the child priorites
  for (auto id : taskIds())
  {
    auto pSubTask = m_pManager->task(id);
    if (nullptr != pSubTask)
    {
      dAutoPriority = std::max<double>(dAutoPriority, pSubTask->autoPriority());
    }
  }

  return dAutoPriority;
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
  auto vTaskIds = property<std::vector<task_id>>("tasks");
  std::set<task_id> ids;
  for (const auto& id : vTaskIds)
  {
    ids.insert(id);
  }
  return ids;
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

  auto vTaskIds = property<std::vector<task_id>>("tasks");
  vTaskIds.push_back(id);
  return setProperty("tasks", vTaskIds);
}

bool Task::removeTask(task_id id)
{
  auto vTaskIds = property<std::vector<task_id>>("tasks");
  auto it = std::find(vTaskIds.begin(), vTaskIds.end(), id);
  if (it != vTaskIds.end())
  {
    vTaskIds.erase(it);
    bool bSuccess = setProperty("tasks", vTaskIds);

    ITask* pTask = m_pManager->task(id);
    if (nullptr != pTask)
    {
      pTask->setParentTask(-1);
    }
    return bSuccess;
  }

  return false;
}

void Task::startWork(const QDateTime& when)
{
  STimeFragment fragment;
  fragment.startTime = when;
  m_vTimingInfo.push_back(fragment);
}

void Task::stopWork(const QDateTime& when)
{
  if (m_vTimingInfo.empty())  { return; }

  if (!m_vTimingInfo.back().stopTime.isValid())
  {
    m_vTimingInfo.back().stopTime = when;
  }
}

bool Task::isWorking() const
{
  if (m_vTimingInfo.empty())  { return false; }

  return !m_vTimingInfo.back().stopTime.isValid();
}

void Task::insertTimeFragment(const QDateTime& start, const QDateTime& end)
{
  startWork(start);
  stopWork(end);

  // TODO: move this code to stopWork so that merging of fragment also works
  // with starting/stopping when providing specific date times.

  // iterate through all time fragments and check for overlaps or fragments
  // in close proximity to each other and fuse themg
  std::sort(m_vTimingInfo.begin(), m_vTimingInfo.end(),
            [](const STimeFragment& lhs, const STimeFragment& rhs)
  {
    return lhs.startTime < rhs.startTime;
  });

  static const int c_iMaxSecsDeltaForFuse = 60;
  auto it = m_vTimingInfo.begin();
  STimeFragment* pLastEl = &*it++;
  while (it != m_vTimingInfo.end())
  {
    if (nullptr != pLastEl)
    {
      if (c_iMaxSecsDeltaForFuse >= pLastEl->stopTime.secsTo(it->startTime))
      {
        pLastEl->stopTime = std::max<QDateTime>(pLastEl->stopTime, it->stopTime);
        it = m_vTimingInfo.erase(it);
      }
      else
      {
        // only if the two elements were not fused should the last element pointer
        // point to the current element.
        pLastEl = &*it;
        ++it;
      }
    }
    else
    {
      ++it;
    }
  }
}

void Task::removeTimeFragment(const QDateTime& start, const QDateTime& end)
{
  auto it = m_vTimingInfo.begin();
  while (it != m_vTimingInfo.end())
  {
    if (it->startTime >= end || it->stopTime <= start)
    {
      // common case: no collision with [start, end]
      ++it;
    }
    else
    {
      if (it->startTime>= start && it->stopTime <= end)
      {
        // the current time fragment lies completely within [start, end]
        // -> remove the fragment
        it = m_vTimingInfo.erase(it);
      }
      else if (it->startTime < start && it->stopTime > end)
      {
        // the time fragment that is to be excluded lies completely within the current fragment
        // -> split the fragment
        auto originalStop = it->stopTime;
        it->stopTime = start;
        STimeFragment newFragment;
        newFragment.startTime = end;
        newFragment.stopTime = originalStop;
        it = m_vTimingInfo.insert(it + 1, newFragment);
      }
      else if (it->startTime < end && it->stopTime > end)
      {
        // start point of current time fragment lies within [start, end]
        // -> chop from the beginning of the fragment
        it->startTime = end;
        ++it;
      }
      else if (it->startTime < start && it->stopTime > start)
      {
        // end point of current fragment lies within [start, end]
        // -> chop from the end of the fragment
        it->stopTime = start;
        ++it;
      }
      else
      {
        assert(false && "all cases should be covered. This code should not be executed.");
      }
    }
  }
}

bool Task::isTrackingTime() const
{
  if (m_vTimingInfo.empty())  { return false; }

  return !m_vTimingInfo.back().stopTime.isValid();
}

std::vector<STimeFragment> Task::timeFragments() const
{
  return m_vTimingInfo;
}

void Task::setTimeFragments(const std::vector<STimeFragment>& vFragments)
{
  m_vTimingInfo = vFragments;
}

std::set<tag_id> Task::tagIds() const
{
  auto vTagIds = property<std::vector<tag_id>>("tags");
  std::set<tag_id> ids;
  for (const auto& id : vTagIds)
  {
    ids.insert(id);
  }
  return ids;
}

bool Task::addTag(tag_id tagId)
{
  auto vTagIds = property<std::vector<tag_id>>("tags");
  auto it = std::find(vTagIds.begin(), vTagIds.end(), tagId);
  if (it == vTagIds.end())
  {
    vTagIds.push_back(tagId);
    return setProperty("tags", vTagIds);
  }
  return false;
}

bool Task::removeTag(tag_id id)
{
  auto vTagIds = property<std::vector<tag_id>>("tags");
  auto it = std::find(vTagIds.begin(), vTagIds.end(), id);
  if (it != vTagIds.end())
  {
    vTagIds.erase(it);
    return setProperty("tags", vTagIds);
  }
  return false;
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

  for (const auto& subTaskId : taskIds())
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

bool Task::removeProperty(const QString& sName)
{
  return m_properties.remove(sName);
}
