#ifndef CONVERSION_BITSET_H
#define CONVERSION_BITSET_H

#include <QString>
#include <bitset>

namespace detail {
  template<int count>
  struct to_string<std::bitset<count>>
  {
    static QString convert(const std::bitset<count>& bs)
    {
      return QString::fromStdString(bs.to_string());
    }
  };

  template<int count>
  struct from_string<std::bitset<count>> {
    static std::bitset<count> convert(const QString& s, bool& bConversionStatus)
    {
      try {
        bConversionStatus = true;
        return std::bitset<count>(s.toStdString());
      }
      catch (std::invalid_argument) {
        bConversionStatus = false;
      }
      return std::bitset<count>{};
    }
  };
}

namespace conversion {
  QString bitsetToWeekDays(const std::bitset<7>& bs)
  {
    if (0b1100000 == bs.to_ulong()) {
      return "every weekend";
    }
    else if (0b0011111 == bs.to_ulong()) {
      return "every workday";
    }

    QStringList l;
    for (int i = 0; i < 7; ++i)
    {
      if (bs[i]) { l.push_back(QDate::shortDayName(i+1)); }
    }
    return QString("every %1").arg(l.join(", "));
  }
}
#endif // CONVERSION_BITSET_H
