#include "conversion.h"

#include <QRegExp>
#include <set>
#include <vector>
#include <functional>

namespace conversion
{
  template<> int fromString<int>(const QString& sVal, bool& bConversionStatus)
  {
    bConversionStatus = false;
    int iVal = sVal.toInt(&bConversionStatus);
    if (bConversionStatus)  { return iVal; }
    return 0;
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

    // try to match natural language input
    /*
in 5 minutes
in 5 years
in 2 days
in 2 weeks
in 2h
in 17 hours
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
    QRegExp relativeToNow("^in ([0-9]+)[\\s]*(\\w*)");

    if (0 == relativeToNow.indexIn(sVal))
    {
      QString sQuantity = relativeToNow.cap(1);
      bool bIsInt(false);
      int iQuantity = sQuantity.toInt(&bIsInt);
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
