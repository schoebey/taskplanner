#include "recurrence.h"

#include <QString>

#include <map>

namespace conversion {
  template<> QString toString(const WeekDays& wd)
  {
    QStringList l;
    for (int f = 0; f <= 7; ++f) {
      if (wd.testFlag(static_cast<EWeekDay>(1 << f))) { l.push_back(QDate::shortDayName(f + 1));}
    }
    return l.join("|");
  }

  template<> WeekDays fromString(const QString& s, bool& bConversionStatus)
  {
    static const std::map<QString, EWeekDay> dayNameLut = {
      {QDate::shortDayName(1), EWeekDay::Monday},
      {QDate::shortDayName(2), EWeekDay::Tuesday},
      {QDate::shortDayName(3), EWeekDay::Wednesday},
      {QDate::shortDayName(4), EWeekDay::Thursday},
      {QDate::shortDayName(5), EWeekDay::Friday},
      {QDate::shortDayName(6), EWeekDay::Saturday},
      {QDate::shortDayName(7), EWeekDay::Sunday},
    };

    WeekDays wd;
    QStringList l = s.split("|");
    for (const QString& sDay : l) {
      auto itDay = dayNameLut.find(sDay);
      if (itDay != dayNameLut.end()) {
        wd.setFlag(itDay->second);
      }
    }

    bConversionStatus = true;
    return wd;
  }


  template<> QString toString(const SRecurrence& r)
  {
    return QString("%1,%2,%3").arg(toString(r.wd)).arg(r.startTime.toString("hh:mm")).arg(conversion::toString(r.duration));
  }

  template<> SRecurrence fromString(const QString& s, bool& bConversionStatus)
  {
    SRecurrence r;
    QStringList l = s.split(",");
    if (l.size() == 3)
    {
      r.wd = fromString<WeekDays>(l[0], bConversionStatus);
      r.startTime = QTime::fromString(l[1], "hh:mm");
      r.duration = fromString<std::chrono::minutes>(l[2], bConversionStatus);
    }
    return r;
  }

  QString recurrenceToDisplayString(const SRecurrence& r) {
    return QString("every %1 at %2, for %3").arg(toString(r.wd)).arg(r.startTime.toString("hh:mm")).arg(conversion::toString(r.duration));
  }

  QString recurrenceVectorToDisplayString(const std::vector<SRecurrence>& vR) {
    QStringList l;
    for (const auto& r : vR) {
      l.push_back(recurrenceToDisplayString(r));
    }
    return l.join(";");
  }
}
