#ifndef CONVERSION_H
#define CONVERSION_H

#include "libtaskmanager.h"

#include <type_traits>

#include <vector>

#include <QString>
#include <QDateTime>
#include <QUrl>
#include <QColor>

namespace conversion
{
  static const QString c_sDateTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";

  namespace fancy
  {
    QString LIBTASKMANAGER toString(const QTime& t);

    QString LIBTASKMANAGER dateToString(const QString& sDate);

    QString LIBTASKMANAGER toString(int iValue, int iDepth = 0);

    int LIBTASKMANAGER toInt(const QString& sNumber, bool* pbStatus, int iStartValue = 0);
  }

  template<typename T>
  typename std::enable_if<!std::is_convertible<T, QString>::value &&
                          !std::is_arithmetic<T>::value, QString>::type
  toString(const T& /*value*/);

  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, QString>::type
  toString(const T& num)
  {
    return QString::number(num);
  }

  template<typename T>
  typename std::enable_if<std::is_convertible<T, QString>::value, QString>::type
  toString(const T& val)
  {
    return QString(val);
  }

  template<typename T>
  typename std::enable_if<!std::is_convertible<QString, T>::value &&
                          !std::is_arithmetic<T>::value, T>::type
  fromString(const QString& /*sVal*/, bool& /*bConversionStatus*/);

  template<typename T>
  typename std::enable_if<std::is_convertible<QString, T>::value, T>::type
  fromString(const QString& sVal, bool& bConversionStatus)
  {
    bConversionStatus = true;
    return T(sVal);
  }
  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, T>::type
  fromString(const QString& sVal, bool& bConversionStatus)
  {
    return T(sVal.toDouble(&bConversionStatus));
  }

  //-- int
  template<> int LIBTASKMANAGER fromString<int>(const QString& sVal, bool& bConversionStatus);

  //-- QDateTime
  template<> QDateTime LIBTASKMANAGER fromString<QDateTime>(const QString& sVal, bool& bConversionStatus);
  QString LIBTASKMANAGER toString(const QDateTime& dt);

  //-- bool
  template<> bool LIBTASKMANAGER fromString<bool>(const QString& sVal, bool& bConversionStatus);
  QString LIBTASKMANAGER toString(bool bVal);

  //-- std::vector<QUrl>
  template<> std::vector<QUrl> LIBTASKMANAGER fromString<std::vector<QUrl>>(const QString& sVal, bool& bConversionStatus);
  QString LIBTASKMANAGER toString(const std::vector<QUrl>& vUrls);

  //-- QColor
  template<> QColor LIBTASKMANAGER fromString<QColor>(const QString& sVal, bool& bConversionStatus);
  QString LIBTASKMANAGER toString(const QColor& c);
}

#endif // CONVERSION_H
