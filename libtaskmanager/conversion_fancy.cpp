#include "conversion_fancy.h"

#include <QTime>

#include <vector>
#include <functional>
#include <cmath>
#include <map>

namespace conversion
{
  namespace fancy
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

      for (size_t i = 0; i < tens.size(); ++i)
      {
        if (sNumberCopy.startsWith(tens[i]))
        {
          iResult += i * 10;
          sNumberCopy.remove(0, tens[i].size());
        }
      }

      for (size_t i = 0; i < ones.size(); ++i)
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

      for (size_t i = 0; i < powers.size(); ++i)
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
}
