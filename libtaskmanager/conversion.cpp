#include "conversion.h"

#include <QRegExp>
#include <QString>
#include <QStringList>
#include <set>
#include <vector>
#include <map>
#include <functional>
#include <cmath>

namespace conversion
{
  bool setDayFromString(QDateTime& dt, const QString& s)
  {
    if (s.isEmpty())  { return true; }

    QRegExp rx(R"(\d{1,2})");
    if (0 <= rx.indexIn(s))
    {
      bool bOk(false);
      int iDay = rx.cap(0).toInt(&bOk);
      if (bOk && 0 < iDay && iDay < dt.date().daysInMonth())
      {
        dt = dt.addDays(iDay - dt.date().day());
        return true;
      }
    }

    return false;
  }

  static const std::set<QString> c_sDateFormats = {c_sDateTimeFormat,
                                                   "yyyy-MM-dd",
                                                   "yy-MM-dd",
                                                   "yyyy MM dd",
                                                   "yy MM dd",
                                                   "dd.MM.yyyy",
                                                   "dd.MM.yy"};

  static const std::set<QString> c_sTimeFormats = {"hh:mm:ss",
                                                   "hh:mm:ss AP",
                                                   "hh:mm:ss ap",
                                                   "hh:mm",
                                                   "hh:mm AP",
                                                   "hh:mm ap"};

  QTime timeFromString(const QString& sVal, bool& bConversionStatus, const QTime& baseTime)
  {
    for (const auto& format : c_sTimeFormats)
    {
      QTime t = QTime::fromString(sVal, format);
      bConversionStatus = t.isValid();
      if (bConversionStatus)
      {
        return t;
      }
    }

    // first, try to match fixed time points
    // e.g. at 7:45 AM
    QRegExp rx(R"((^.*)( on | at |@)(.*))");
    if (0 == rx.indexIn(sVal))
    {
      // we have an absolute time/date point. Parse it!
      QString sFirstPart = rx.cap(1);
      QString sSecondPart = rx.cap(3);

      if (sFirstPart.isEmpty() ^ sSecondPart.isEmpty())
      {
        return timeFromString(sFirstPart.isEmpty() ? sSecondPart : sFirstPart, bConversionStatus, baseTime);
      }
    }
    else
    {
      rx = QRegExp(R"(^(on |at |@)\s*(.*)$)");
      if (0 == rx.indexIn(sVal))
      {
        QString sCap = rx.cap(2);
        if (!sCap.isEmpty())
        {
          return timeFromString(sCap, bConversionStatus, baseTime);
        }
      }
      else
      {
        QStringList parts = sVal.split(" ");
        if (2 <= parts.size())
        {
          QTime t = timeFromString(parts[0], bConversionStatus, baseTime);
          if (bConversionStatus)
          {
            return t;
          }
        }
      }
    }



    // try to match natural language input
    /*
      in two seconds
      in 20 s
      in 10 secs
      in 5 minutes
      in 5 years
      in 2 days
      in 2 weeks
      in 2h
      in 17 hours
      in two and a half weeks
      in 5mins
      in a hundred years
    */
    std::function<void(QDateTime&, const QTime&)> setToNextTime = [](QDateTime& dt, const QTime& t)
    {
      if (dt.time() > t)
      {
        dt = dt.addDays(1);
      }
      dt.setTime(t);
    };
    std::function<void(QTime&, int)> addSecs = [](QTime& t, int iOffset) { t = t.addSecs(iOffset); };
    std::function<void(QTime&, int)> addMins = [](QTime& t, int iOffset) { t = t.addSecs(60 * iOffset); };
    std::function<void(QTime&, int)> addHours = [](QTime& t, int iOffset) { t = t.addSecs(3600 * iOffset); };
    std::vector<std::pair<QRegExp, std::function<void(QTime&, int)>>> addUnitQuantity;
    addUnitQuantity.push_back(std::make_pair(QRegExp("s(?:ec(?:ond)?)?(?:s)?"), addSecs));
    addUnitQuantity.push_back(std::make_pair(QRegExp("m(?:in(?:ute)?)?(?:s)?"), addMins));
    addUnitQuantity.push_back(std::make_pair(QRegExp("h(?:our)?(?:s)?"), addHours));
    QRegExp relativeToNow("^in ((?:\\S*\\s*)*)\\s+(\\S+){1}$");

    if (0 == relativeToNow.indexIn(sVal))
    {
      QString sQuantity = relativeToNow.cap(1);
      bool bIsInt(false);
      int iQuantity = fromString<int>(sQuantity, bIsInt);
      if (bIsInt)
      {
        QString sUnit = relativeToNow.cap(2);
        for (const auto& el : addUnitQuantity)
        {
          if (el.first.exactMatch(sUnit))
          {
            QTime t = baseTime;
            el.second(t, iQuantity);
            bConversionStatus = true;
            return t;
          }
        }
      }
    }


    return QTime();
  }

  template<>
  QTime fromString<QTime>(const QString& sVal, bool& bConversionStatus)
  {
    return timeFromString(sVal, bConversionStatus, QTime::currentTime());
  }

  QString toString(const QTime& t)
  {
    return t.toString(c_sTimeFormat);
  }

  QDateTime dateTimeFromString(const QString& sVal, bool& bConversionStatus,
                               const QDateTime& baseDateTime)
  {
    for (const auto& format : c_sDateFormats)
    {
      QDateTime dt = QDateTime::fromString(sVal, format);
      bConversionStatus = dt.isValid();
      if (bConversionStatus)  { return dt; }
    }

    for (const auto& format : c_sTimeFormats)
    {
      QTime t = QTime::fromString(sVal, format);
      bConversionStatus = t.isValid();
      if (bConversionStatus)
      {
        QDateTime copy(baseDateTime);
        copy.setTime(t);
        return copy;
      }
    }

    // if standard Qt date format didn't match, try custom matching
    // e.g. aug 25th (without year would match the next occurrence)

    // try to separate date and time

    // first, try to match fixed date or time points
    // e.g. at 7:45 AM
//    on august 25th
//    on august 25th at 7:45 PM
    QRegExp rx(R"((^.*)( on | at |@)(.*))");
    if (0 == rx.indexIn(sVal))
    {
      // we have an absolute time/date point. Parse it!
      QString sFirstPart = rx.cap(1);
      QString sSecondPart = rx.cap(3);

      if (sFirstPart.isEmpty() ^ sSecondPart.isEmpty())
      {
        return dateTimeFromString(sFirstPart.isEmpty() ? sSecondPart : sFirstPart, bConversionStatus, baseDateTime);
      }

      QDateTime dt = dateTimeFromString(sFirstPart, bConversionStatus, baseDateTime);
      if (bConversionStatus)
      {
        return dateTimeFromString(sSecondPart, bConversionStatus, dt);
      }
    }
    else
    {
      rx = QRegExp(R"(^(on |at |@)\s*(.*)$)");
      if (0 == rx.indexIn(sVal))
      {
        QString sCap = rx.cap(2);
        if (!sCap.isEmpty())
        {
          return dateTimeFromString(sCap, bConversionStatus, baseDateTime);
        }
      }
      else
      {
        QStringList parts = sVal.split(" ");
        if (2 <= parts.size())
        {
          QDateTime dt = dateTimeFromString(parts[0], bConversionStatus, baseDateTime);
          if (bConversionStatus)
          {
            QDateTime dt2 = dateTimeFromString(parts[1], bConversionStatus, dt);
            if (bConversionStatus)
            {
              return dt2;
            }
          }
        }
      }
    }



    // try to match natural language input
    /*
in two seconds
in 20 s
in 10 secs
in 5 minutes
in 5 years
in 2 days
in 2 weeks
in 2h
in 17 hours
in two and a half weeks
in 5mins
in a hundred years
*/
    std::function<void(QDateTime&, const QTime&)> setToNextTime = [](QDateTime& dt, const QTime& t)
    {
      if (dt.time() > t)
      {
        dt = dt.addDays(1);
      }
      dt.setTime(t);
    };

    std::function<void(QDateTime&, int, int, int)> setToNextDate = [](QDateTime& dt, int iYear, int iMonth, int iDay)
    {
      QDate date(dt.date());
      if (-1 == iYear)
      {
        if (-1 != iMonth && iMonth < date.month()) { iYear = date.year() + 1; } else { iYear = date.year(); }
      }

      if (-1 == iMonth)
      {
        if (-1 != iDay && iDay < date.day()) { iMonth = date.month() + 1; } else { iMonth = date.month(); }
      }


      date = QDate(iYear, iMonth, iDay);

      dt.setDate(date);
    };
    std::function<void(QDateTime&, int)> addSecs = [](QDateTime& dt, int iOffset) { dt = dt.addSecs(iOffset); };
    std::function<void(QDateTime&, int)> addMins = [](QDateTime& dt, int iOffset) { dt = dt.addSecs(60 * iOffset); };
    std::function<void(QDateTime&, int)> addHours = [](QDateTime& dt, int iOffset) { dt = dt.addSecs(3600 * iOffset); };
    std::function<void(QDateTime&, int)> addDays = [](QDateTime& dt, int iOffset) { dt = dt.addDays(iOffset); };
    std::function<void(QDateTime&, int)> addWeeks = [](QDateTime& dt, int iOffset) { dt = dt.addDays(7 * iOffset); };
    std::function<void(QDateTime&, int)> addMonths = [](QDateTime& dt, int iOffset) { dt = dt.addMonths(iOffset); };
    std::function<void(QDateTime&, int)> addYears = [](QDateTime& dt, int iOffset) { dt = dt.addYears(iOffset); };
    std::vector<std::pair<QRegExp, std::function<void(QDateTime&, int)>>> addUnitQuantity;
    addUnitQuantity.push_back(std::make_pair(QRegExp("s(?:ec(?:ond)?)?(?:s)?"), addSecs));
    addUnitQuantity.push_back(std::make_pair(QRegExp("m(?:in(?:ute)?)?(?:s)?"), addMins));
    addUnitQuantity.push_back(std::make_pair(QRegExp("h(?:our)?(?:s)?"), addHours));
    addUnitQuantity.push_back(std::make_pair(QRegExp("d(?:ay)?(?:s)?"), addDays));
    addUnitQuantity.push_back(std::make_pair(QRegExp("w(?:eek)?(?:s)?"), addWeeks));
    addUnitQuantity.push_back(std::make_pair(QRegExp("month(?:s)?"), addMonths));
    addUnitQuantity.push_back(std::make_pair(QRegExp("y(?:ear)?(?:s)?"), addYears));
    QRegExp relativeToNow("^in ((?:\\S*\\s*)*)\\s+(\\S+){1}$");

    if (0 == relativeToNow.indexIn(sVal))
    {
      QString sQuantity = relativeToNow.cap(1);
      QString sTest = relativeToNow.cap(2);
      QString sB = relativeToNow.cap(3);
      QString sWhole = relativeToNow.cap(0);
      bool bIsInt(false);
      int iQuantity = fromString<int>(sQuantity, bIsInt);
      if (bIsInt)
      {
        QString sUnit = relativeToNow.cap(2);
        for (const auto& el : addUnitQuantity)
        {
          if (el.first.exactMatch(sUnit))
          {
            QDateTime dt = baseDateTime;
            el.second(dt, iQuantity);
            bConversionStatus = true;
            return dt;
          }
        }
      }
    }


    // 2nd type of offset: 'next' instance of
    // "next week", "next tuesday", "next full moon", etc.
    // will set the date to the end of the provided unit
    // e.g. next day will set the date to the end of the next day
    // (end of work day and end of week will have to be configured,
    // like 5pm, friday
    static const std::vector<QRegExp> weekdays = {QRegExp("mon(?:day)?"),
                                                  QRegExp("tue(?:sday)?"),
                                                  QRegExp("wed(?:nesday)?"),
                                                  QRegExp("thu(?:rsday)?"),
                                                  QRegExp("fri(?:day)?"),
                                                  QRegExp("sat(?:urday)?"),
                                                  QRegExp("sun(?:day)?")};
    static const std::vector<QRegExp> months = {QRegExp("jan(?:uary)?"),
                                                QRegExp("feb(?:ruary)?"),
                                                QRegExp("mar(?:ch)?"),
                                                QRegExp("apr(?:il)?"),
                                                QRegExp("may"),
                                                QRegExp("jun(?:e)?"),
                                                QRegExp("jul(?:y)?"),
                                                QRegExp("aug(?:ust)?"),
                                                QRegExp("sep(?:t(?:empber)?)?"),
                                                QRegExp("oct(?:ober)?"),
                                                QRegExp("nov(?:ember)?"),
                                                QRegExp("dec(?:ember)?")};
    // keywords, e.g. 'tomorrow', 'yesterday', 'noon', 'midnight', etc.
    std::vector<std::pair<QRegExp, std::function<void(QDateTime&)>>> addFixQuantityForKeywords;
    addFixQuantityForKeywords.push_back(std::make_pair(QRegExp("tomorrow"), std::bind(addDays, std::placeholders::_1, 1)));
    addFixQuantityForKeywords.push_back(std::make_pair(QRegExp("yesterday"), std::bind(addDays, std::placeholders::_1, -1)));
    addFixQuantityForKeywords.push_back(std::make_pair(QRegExp("midnight"), std::bind(setToNextTime, std::placeholders::_1, QTime(0, 0))));
    addFixQuantityForKeywords.push_back(std::make_pair(QRegExp("noon"), std::bind(setToNextTime, std::placeholders::_1, QTime(12, 0))));
    addFixQuantityForKeywords.push_back(std::make_pair(QRegExp("christmas|xmas"), std::bind(setToNextDate, std::placeholders::_1, -1, 12, 25)));
    addFixQuantityForKeywords.push_back(std::make_pair(QRegExp("new year's eve"), std::bind(setToNextDate, std::placeholders::_1, -1, 12, 31)));
    addFixQuantityForKeywords.push_back(std::make_pair(QRegExp("new year"), std::bind(setToNextDate, std::placeholders::_1, -1, 1, 1)));

    QDateTime dt = baseDateTime;
    QRegExp nextInstance(R"(^(next |this )?(\D*))");
    if (0 == nextInstance.indexIn(sVal))
    {
      // TODO: differentiate this/next (-> next occurrence,or the one after)
      QString sInst = nextInstance.cap(1);
      QString sType = nextInstance.cap(2);

      // unit (see above)?
      for (const auto& el : addUnitQuantity)
      {
        if (el.first.exactMatch(sType))
        {
          // for now, we will calculate the same timepoint in the next unit
          // (e.g. the same time tomorrow, same day next week, etc.)
          // ideal would be the last feasible timepoint in the desired unit
          // (e.g. friday, 5pm next week, 5pm tomorrow, last day next month)
          el.second(dt, 1);
          bConversionStatus = true;
          return dt;
        }
      }

      // weekday (mon-sun)? for this, string format "on XY" would also work
      for (size_t i = 0; i < weekdays.size(); ++i)
      {
        if (0 == weekdays[i].indexIn(sType))
        {
          int iCurrentDay = dt.date().dayOfWeek() - 1;
          int iOffset = i - iCurrentDay;
          if (iOffset <= 0)  iOffset += 7;
          dt = dt.addDays(iOffset);
          bConversionStatus = true;
          return dt;
        }
      }


      // month (jan-dec)?
      for (size_t i = 0; i < months.size(); ++i)
      {
        if (0 <= months[i].indexIn(sType))
        {
          int iCurrentMonth = dt.date().month() - 1;
          int iOffset = i - iCurrentMonth;
          if (iOffset <= 0)  iOffset += 12;
          dt = dt.addMonths(iOffset);

          QString sRest = sVal;
          sRest.remove(nextInstance.cap(0));

          // try to extract day number from the rest
          bConversionStatus = setDayFromString(dt, sRest);
          return dt;
        }
      }


      // named days (holidays, e.g. 'christmas')?
      // TODO: could benefit from working implementation of 'dateFromTrivialString("dec 24th")'
      for (const auto& el : addFixQuantityForKeywords)
      {
        // if keyword finds a match in the string, execute its function
        // After a match was found, remove the whole capture and call dateFromString with the rest
        // This way, constructs like "tomorrow at 4:53" should be possible...
        int iIndex = el.first.indexIn(sType);
        if (-1 != iIndex)
        {
          el.second(dt);
          QString sModifiedVal(sType);
          sModifiedVal.remove(iIndex, el.first.matchedLength());

          if (!sModifiedVal.isEmpty())
          {
            return dateTimeFromString(sModifiedVal, bConversionStatus, dt);
          }
          else
          {
            bConversionStatus = true;
            return dt;
          }
        }
      }
    }


    return QDateTime();
  }

  template<> QDateTime fromString<QDateTime>(const QString& sVal, bool& bConversionStatus)
  {
    return dateTimeFromString(sVal, bConversionStatus, QDateTime::currentDateTime());
  }

  QString toString(const QDateTime& dt)
  {
    return dt.toString(c_sDateTimeFormat);
  }

  template<>
  bool fromString<bool>(const QString& sVal, bool& bConversionStatus)
  {
    bool bRv = 1 == sVal.toInt(&bConversionStatus);
    if (!bConversionStatus)
    {
      bConversionStatus = 0 == sVal.compare("true", Qt::CaseInsensitive) ||
                          0 == sVal.compare("false", Qt::CaseInsensitive);
      return 0 == sVal.compare("true", Qt::CaseInsensitive);
    }

    return bRv;
  }

  QString toString(bool bVal)
  {
    return bVal ? "true" : "false";
  }

  template<>
  QColor fromString<QColor>(const QString& sVal, bool& bConversionStatus)
  {
    QColor c(sVal);
    bConversionStatus = c.isValid();
    return c;
  }

  QString toString(const QColor& c)
  {
    return c.name(QColor::HexArgb);
  }
}
