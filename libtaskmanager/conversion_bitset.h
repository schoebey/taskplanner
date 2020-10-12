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

#endif // CONVERSION_BITSET_H
