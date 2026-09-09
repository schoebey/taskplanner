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
  QDateTime cycleStart; // only used when repeatMode == Recurring: fixed moment the cycle began,
                        // set once at creation/first-set time; NOT recomputed relative to "today"
  bool bEnabled = true;

  bool operator==(const SReminder& other) const
  {
    return intervalUnit == other.intervalUnit &&
        iIntervalCount == other.iIntervalCount &&
        triggerTime == other.triggerTime &&
        repeatMode == other.repeatMode &&
        dueDateTime == other.dueDateTime &&
        cycleStart == other.cycleStart &&
        bEnabled == other.bEnabled;
  }
};

#endif // REMINDER_H
