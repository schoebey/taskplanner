#include "conversion.h"

#include <QRegExp>
#include <set>
#include <vector>
#include <map>
#include <functional>
#include <cmath>

namespace conversion
{
  int stringToInt(const QString& sNumber, bool* pbStatus, int iStartValue)
  {
    QString sNumberCopy(sNumber.simplified());
    sNumberCopy.remove(" ");
    static const std::vector<QString> one_variants = {"one", "a", "an"};
    static const std::vector<QString> ones = {"-", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten",
                                              "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen",
                                              "eighteen", "nineteen"};
    static const std::vector<QString> tens = {"-", "-", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};
    static const QString hundred = "hundred";
    static const std::vector<QString> powers = {"-", "thousand", "million", "billion", "trillion"};
    static const QString sAnd = "and";

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
      return stringToInt(sNumberCopy, pbStatus, iResult);
    }

    if (nullptr != pbStatus)
    {
      *pbStatus = sNumberCopy.isEmpty();
    }

    return iResult;
  }


  template<> int fromString<int>(const QString& sVal, bool& bConversionStatus)
  {
    bConversionStatus = false;
    int iVal = sVal.toInt(&bConversionStatus);
    if (bConversionStatus)  { return iVal; }
    return stringToInt(sVal, &bConversionStatus);
  }

  static const QString c_sDateTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";
  static const std::set<QString> c_sDateTimeFormats = {c_sDateTimeFormat,
                                                       "yyyy-MM-dd hh:mm:ss",
                                                       "yyyy-MM-dd hh:mm",
                                                       "yyyy-MM-dd",
                                                       "yy-MM-dd",
                                                       "yyyy MM dd",
                                                       "yy MM dd",
                                                       "dd.MM.yyyy",
                                                       "dd.MM.yy"};
  template<> QDateTime fromString<QDateTime>(const QString& sVal, bool& bConversionStatus)
  {
    for (const auto& format : c_sDateTimeFormats)
    {
      QDateTime dt = QDateTime::fromString(sVal, format);
      bConversionStatus = dt.isValid();
      if (bConversionStatus)  { return dt; }
    }

    // if standard Qt date format didn't match, try custom matching
    // e.g. aug 25th (without year would match the next occurrence)
    // TODO: implement

    // try to match natural language input
    /*
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
    std::function<void(QDateTime&, int)> addMins = [](QDateTime& dt, int iOffset) { dt = dt.addSecs(60 * iOffset); };
    std::function<void(QDateTime&, int)> addHours = [](QDateTime& dt, int iOffset) { dt = dt.addSecs(3600 * iOffset); };
    std::function<void(QDateTime&, int)> addDays = [](QDateTime& dt, int iOffset) { dt = dt.addDays(iOffset); };
    std::function<void(QDateTime&, int)> addWeeks = [](QDateTime& dt, int iOffset) { dt = dt.addDays(7 * iOffset); };
    std::function<void(QDateTime&, int)> addMonths = [](QDateTime& dt, int iOffset) { dt = dt.addMonths(iOffset); };
    std::function<void(QDateTime&, int)> addYears = [](QDateTime& dt, int iOffset) { dt = dt.addYears(iOffset); };
    std::vector<std::pair<QRegExp, std::function<void(QDateTime&, int)>>> addUnitQuantity;
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
            QDateTime dt = QDateTime::currentDateTime();
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
                                                QRegExp("may"),
                                                QRegExp("jun(?:e)?"),
                                                QRegExp("jul(?:y)?"),
                                                QRegExp("aug(?:ust)?"),
                                                QRegExp("sep(?:t(?:empber)?)?"),
                                                QRegExp("oct(?:ober)?"),
                                                QRegExp("nov(?:ember)?"),
                                                QRegExp("dec(?:ember)?")};
    static const std::map<QRegExp, QString> namedDates = {{QRegExp("xmas|christmas"), "Dec 24th"}};
    QDateTime dt = QDateTime::currentDateTime();
    QRegExp nextInstance("^next (\\S*\\s*)");
    if (0 == nextInstance.indexIn(sVal))
    {
      QString sType = nextInstance.cap(1);

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
        if (0 == months[i].indexIn(sType))
        {
          int iCurrentMonth = dt.date().month() - 1;
          int iOffset = i - iCurrentMonth;
          if (iOffset <= 0)  iOffset += 12;
          dt = dt.addMonths(iOffset);
          bConversionStatus = true;
          return dt;
        }
      }


      // named days (holidays, e.g. 'christmas')?
      // TODO: could benefit from working implementation of 'dateFromTrivialString("dec 24th")'
    }

    return QDateTime();
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

}
