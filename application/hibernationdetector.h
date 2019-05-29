#ifndef HIBERNATIONDETECTOR_H
#define HIBERNATIONDETECTOR_H

#include <QDateTime>
#include <QObject>

class HibernationDetector : public QObject
{
  Q_OBJECT
public:
  explicit HibernationDetector(QObject *parent = nullptr);
  ~HibernationDetector() = default;

  void setMinDetectionDeltaT(qint64 iSecondsDeltaT);
  qint64 minDetectionDeltaT() const;

signals:
  void wokeUpFromHibernation(const QDateTime& sleepTime,
                             const QDateTime& wakeUpTime);

public slots:

private:
  void timerEvent(QTimerEvent* pEvent);

private:
  QDateTime m_previousSample;
  qint64 m_iDetectionDeltaTSeconds;
};

#endif // HIBERNATIONDETECTOR_H
