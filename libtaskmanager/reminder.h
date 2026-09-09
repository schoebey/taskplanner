#ifndef REMINDER_H
#define REMINDER_H

#include <QTime>
#include <QDateTime>

enum class EReminderIntervalUnit
{
  Hours,
  Days
};

enum class EReminderRepeatMode
{
  Recurring,
  SingleShot
};

struct SReminder
{
  EReminderIntervalUnit intervalUnit = EReminderIntervalUnit::Hours;
  int iIntervalCount = 0;
  QTime triggerTime;
  EReminderRepeatMode repeatMode = EReminderRepeatMode::Recurring;
  QDateTime dueDateTime; // only used when repeatMode == SingleShot: absolute one-time fire moment

  bool operator==(const SReminder& other) const
  {
    return intervalUnit == other.intervalUnit &&
        iIntervalCount == other.iIntervalCount &&
        triggerTime == other.triggerTime &&
        repeatMode == other.repeatMode &&
        dueDateTime == other.dueDateTime;
  }
};

#endif // REMINDER_H
