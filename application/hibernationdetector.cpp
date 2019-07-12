#include "hibernationdetector.h"

HibernationDetector::HibernationDetector(QObject *parent)
  : QObject(parent),
    m_previousSample(QDateTime::currentDateTime()),
    m_iDetectionDeltaTSeconds(300),
    m_bEnabled(true)
{
  startTimer(30000);
}

void HibernationDetector::setMinDetectionDeltaT(qint64 iSecondsDeltaT)
{
  m_iDetectionDeltaTSeconds = iSecondsDeltaT;
}

qint64 HibernationDetector::minDetectionDeltaT() const
{
  return m_iDetectionDeltaTSeconds;
}

void HibernationDetector::setEnabled(bool bEnabled)
{
  m_bEnabled = bEnabled;
}

void HibernationDetector::timerEvent(QTimerEvent* /*pEvent*/)
{
  QDateTime now = QDateTime::currentDateTime();

  if (m_bEnabled)
  {
    auto deltaSeconds = m_previousSample.secsTo(now);

    if (deltaSeconds > m_iDetectionDeltaTSeconds)
    {
      emit wokeUpFromHibernation(m_previousSample, now);
    }
  }

  m_previousSample = now;
}
