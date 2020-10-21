#ifndef CONVERSION_CHRONO_H
#define CONVERSION_CHRONO_H

#include <QString>
#include <chrono>

namespace detail {
//  duration<     long, ratio<  60> >

  template<class R, class P>
  struct to_string<std::chrono::duration<R, P>>
  {
    static QString convert(const std::chrono::duration<R, P>& d)
    {
      return QString::number(d.count());
    }
  };

  template<class R, class P>
  struct from_string<std::chrono::duration<R, P>> {
    static std::chrono::duration<R, P> convert(const QString& s, bool& bConversionStatus)
    {
      try {
        bConversionStatus = true;
        int iCount = s.toInt(&bConversionStatus);
        if (bConversionStatus) {
          return std::chrono::duration<R, P>(iCount);
        }
      }
      catch (std::invalid_argument) {
        bConversionStatus = false;
      }
      return std::chrono::duration<R, P>{};
    }
  };
}

namespace conversion {
  namespace chrono {
    template<class R, class P> QString
    toDisplayString(const std::chrono::duration<R, P>& val)
    {
      int iCount = val.count();

      int iMins = iCount;
      int iHours = iMins / 60;
      int iDays = iHours / 24;

      iMins %= 60;
      iHours %= 24;

      QStringList list;
      if (iDays > 0) list.push_back(QString(iDays == 1 ? "%1 day" : "%1 days").arg(iDays));
      if (iHours > 0) list.push_back(QString(iHours == 1 ? "%1 hour" : "%1 hours").arg(iHours));
      if (iMins > 0) list.push_back(QString(iMins == 1 ? "%1 minute" : "%1 minutes").arg(iMins));

      return list.size() == 3 ?
            QString("%1, %2 and %3").arg(list[0]).arg(list[1]).arg(list[2]) :
            list.join(" and ");
    }

    template<typename T> QString
    toDisplayString(const T& val)
    {
      return conversion::chrono::toDisplayString<typename T::rep, typename T::period>(val);
    }
  }
}

#endif // CONVERSION_CHRONO_H
