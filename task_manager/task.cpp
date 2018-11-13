#include "task.h"

Task::Task()
{

}

QString Task::name() const
{
  return m_sName;
}

void Task::setName(const QString& sName)
{
  m_sName = sName;
}

QString Task::description() const
{
  return m_sDescription;
}

void Task::setDescription(const QString& sDescription)
{
  m_sDescription = sDescription;
}

SPriority Task::priority() const
{
  return m_priority;
}

void Task::setPriority(const SPriority& priority)
{
  m_priority = priority;
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

  m_vTimingInfo.back().stopTime = QDateTime::currentDateTime();
}
