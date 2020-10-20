#ifndef RECURRENCE_H
#define RECURRENCE_H

#include "conversion.h"
#include "conversion_chrono.h"

#include <QFlags>

enum class EWeekDay {
  None = 0,
  Monday = 1,
  Tuesday = 2,
  Wednesday = 4,
  Thursday = 8,
  Friday = 16,
  Saturday = 32,
  Sunday = 64
};
Q_DECLARE_FLAGS(WeekDays, EWeekDay)

struct SRecurrence {
  bool operator==(const SRecurrence& other) const {
    return wd == other.wd &&
        startTime == other.startTime &&
        duration == other.duration;
  }
  WeekDays wd;
  QTime startTime;
  std::chrono::minutes duration;
};


namespace conversion {
  template<> QString toString(const WeekDays& wd);

  template<> WeekDays fromString(const QString& s, bool& bConversionStatus);

  template<> QString toString(const SRecurrence& r);

  template<> SRecurrence fromString(const QString& s, bool& bConversionStatus);

  QString recurrenceToDisplayString(const SRecurrence& r);

  QString recurrenceVectorToDisplayString(const std::vector<SRecurrence>& vR) ;
}


#endif // RECURRENCE_H
