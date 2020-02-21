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
  static const std::vector<QString> one_variants = {"one", "a", "an"};
  static const std::vector<QString> ones = {"-", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten",
                                            "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen",
                                            "eighteen", "nineteen"};
  static const std::vector<QString> tens = {"-", "-", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};
  static const QString hundred = "hundred";
  static const std::vector<QString> powers = {"-", "thousand", "million", "billion", "trillion"};
  static const QString sAnd = "and";

  static const std::vector<QString> negativePrefixes = {"minus", "negative"};

  namespace fancy
  {
    QString toString(const QTime& t)
    {
      if (t.hour() == 12)
      {
        return "noon";
      }
      else if (t.hour() == 0)
      {
        return "midnight";
      }
      else
      {
        return t.toString("hh:mm");
      }
    }

    QString dateToString(const QString& sDate)
    {
      QDateTime dt = QDateTime::fromString(sDate, conversion::c_sDateTimeFormat);

      QString sText;

      // determine the delta to the current date time
      QDateTime now = QDateTime::currentDateTime();

      // due today
      qint64 iMsecsTo = now.msecsTo(dt);
      qint64 iSecsTo = static_cast<int>(floor(iMsecsTo / 1000. + 0.5));
      qint64 iHoursTo = static_cast<int>(floor(iMsecsTo / 3600000. + 0.5));
      qint64 iMinsTo = static_cast<int>(floor(iMsecsTo / 60000. + 0.5)) - iHoursTo * 3600000;
      qint64 iDaysTo = now.daysTo(dt);
      qint64 iWeeksTo = static_cast<int>(floor(iDaysTo / 7. + 0.5));
      qint64 iMonthsTo = static_cast<int>(floor(iWeeksTo / 4. + 0.5));
      qint64 iYearsTo = static_cast<int>(floor(iDaysTo / 365. + 0.5));
      std::vector<qint64> viCounters = {iYearsTo, iMonthsTo, iWeeksTo, iDaysTo, iHoursTo, iMinsTo, iSecsTo};

      enum class Type
      {
        eYears = 0,
        eMonths,
        eWeeks,
        eDays,
        eHours,
        eMinutes,
        eSeconds
      };

      auto multiplicity = [](const QString& sSingular, const QString& sPlural, qint64 iCounter)
      {
        return iCounter > 1 ? sPlural : sSingular;
      };
      auto multiplicityOf = [&](const QString& sSingular, const QString& sPlural, qint64 iCounter)
      {
        return QString("%1 %2").arg((iCounter <= 10) ? toString(iCounter) : QString::number(iCounter)).arg(multiplicity(sSingular, sPlural, iCounter));
      };
      auto in = [&](const QString& sSingular, const QString& sPlural, qint64 iCounter)
      {
        return QString("in %1").arg(multiplicityOf(sSingular, sPlural, iCounter));
      };

      typedef std::function<QString(qint64)> tFnToString;
      std::map<Type, tFnToString> mapping;
      mapping[Type::eYears]   = [&](qint64 i){ return in("year", "years", i); };
      mapping[Type::eMonths]  = [&](qint64 i){ return in("month", "months", i); };
      mapping[Type::eWeeks]   = [&](qint64 i){ return in("week", "weeks", i); };
      mapping[Type::eDays]    = [&](qint64 i){ return in("day", "days", i); };
      mapping[Type::eHours]   = [&](qint64 i){ return in("hour", "hours", i); };
      mapping[Type::eMinutes] = [&](qint64 i){ return in("minute", "minutes", i); };
      mapping[Type::eSeconds] = [&](qint64 i){ return in("second", "seconds", i); };

      qint64* piCounter = &viCounters[0];
      for (const auto& el : mapping)
      {
        if (0 < *piCounter)
        {
          sText = el.second(*piCounter);
          break;
        }
        else
        {
          ++piCounter;
        }
      }


      if (0 == iDaysTo && 4 <= iHoursTo)
      {
        sText = QString("at %1").arg(dt.time().toString("hh:mm"));
      }
      else if (0 > iDaysTo)
      {
        // in the past
        sText = "in the past";
      }
      else if (1 == iDaysTo)
      {
        // due tomorrow
        int iHour = dt.time().hour();
        sText = QString("tomorrow %1 at %2")
            .arg(0 < iHour ? (12 > iHour ? "morning" : (12 < iHour ? (16 < iHour ? "evening" : "afternoon") : "")) : "")
            .arg(toString(dt.time()));
      }


      if (sText.isEmpty())
      {
        return dt.toString("yyyy-MM-dd hh:mm");
      }
      else
      {
        return sText;
      }
    }

    QString toString(int iValue, int iDepth)
    {
      // decode pairs of three
      int iCurrentPack = iValue % 1000;
      int iRest = iValue / 1000;

      int iOnes = iCurrentPack % 10;
      int iTens = (iCurrentPack / 10) % 10;
      if (1 == iTens)
      {
        iOnes = iCurrentPack % 100;
        iTens = 0;
      }
      int iHundreds = (iCurrentPack / 100) % 10;

      QString sValue;
      if (iHundreds > 0)  { sValue += ones[iHundreds] + hundred; }
      if ((iTens > 0 || iOnes > 0) && !sValue.isEmpty())  { sValue += "and"; }
      if (iTens > 0)  { sValue += tens[iTens]; }
      if (iOnes > 0)  { sValue += ones[iOnes]; }

      if (0 < iDepth)
      {
        sValue += powers[iDepth];
      }

      if (iRest != 0)
      {
        QString sAnd = (!sValue.isEmpty() && iHundreds == 0) ? "and" : "";
        sValue = toString(iRest, iDepth + 1) + sAnd + sValue;
      }

      return sValue;
    }

    int toInt(const QString& sNumber, bool* pbStatus, int iStartValue)
    {
      QString sNumberCopy(sNumber.simplified().toLower());
      sNumberCopy.remove(" ");


      bool bNumberIsNegative = false;
      for (const QString& sNegativePrefix : negativePrefixes)
      {
        if (sNumberCopy.startsWith(sNegativePrefix))
        {
          bNumberIsNegative ^= true;
          sNumberCopy.remove(0, sNegativePrefix.size());
        }
      }


      int iResult = 0;
      if (sNumberCopy.startsWith(sAnd))
      {
        sNumberCopy.remove(0, sAnd.size());
      }

      for (const auto& one : one_variants)
      {
        if (sNumberCopy.startsWith(one))
        {
          iResult += 1;
          sNumberCopy.remove(0, one.size());
        }
      }

      for (int i = 0; i < tens.size(); ++i)
      {
        if (sNumberCopy.startsWith(tens[i]))
        {
          iResult += i * 10;
          sNumberCopy.remove(0, tens[i].size());
        }
      }

      for (int i = 0; i < ones.size(); ++i)
      {
        if (sNumberCopy.startsWith(ones[i]))
        {
          iResult += i;
          sNumberCopy.remove(0, ones[i].size());
        }
      }

      if (sNumberCopy.startsWith(hundred))
      {
        iResult *= 100;
        sNumberCopy.remove(0, hundred.size());
      }

      for (int i = 0; i < powers.size(); ++i)
      {
        if (sNumberCopy.startsWith(powers[i]))
        {
          iResult *= std::pow(1000, i);
          sNumberCopy.remove(0, powers[i].size());
        }
      }

      iResult += iStartValue;
      if (iStartValue != iResult)
      {
        return (bNumberIsNegative ? -1 : 1) * toInt(sNumberCopy, pbStatus, iResult);
      }

      if (nullptr != pbStatus)
      {
        *pbStatus = sNumberCopy.isEmpty();
      }

      return bNumberIsNegative ? -iResult : iResult;
    }
  }

  template<> int fromString<int>(const QString& sVal, bool& bConversionStatus)
  {
    bConversionStatus = false;
    int iVal = sVal.toInt(&bConversionStatus);
    if (bConversionStatus)  { return iVal; }
    return fancy::toInt(sVal, &bConversionStatus);
  }

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
        dt.addDays(1);
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
      for (int i = 0; i < weekdays.size(); ++i)
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
      for (int i = 0; i < months.size(); ++i)
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
  std::vector<QUrl> fromString<std::vector<QUrl> >(const QString& sVal, bool& bConversionStatus)
  {
    std::vector<QUrl> vUrls;

    auto list = sVal.split("|");
    for (const auto& el : list)
    {
      if (!el.isEmpty())
      {
        vUrls.push_back(QUrl(el));
      }
    }

    bConversionStatus = true;

    return vUrls;
  }

  QString toString(const std::vector<QUrl>& vUrls)
  {
    QStringList list;

    for (const auto& el : vUrls)
    {
      list.append(el.toString());
    }

    return list.join("|");
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
