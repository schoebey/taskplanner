#include "conversion.h"

#include <set>

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
