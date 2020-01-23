#ifndef TIMEFRAGMENT_H
#define TIMEFRAGMENT_H

#include <QDateTime>

struct STimeFragment
{
  QDateTime startTime;
  QDateTime stopTime;

  bool operator==(const STimeFragment& other) const
  {
    return startTime == other.startTime &&
        stopTime == other.stopTime;
  }
};

#endif // TIMEFRAGMENT_H
