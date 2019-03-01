#ifndef CONVERSION_H
#define CONVERSION_H

#include <type_traits>

#include <vector>

#include <QString>
#include <QDateTime>
#include <QUrl>

namespace conversion
{
  int stringToInt(const QString& sNumber, bool* pbStatus = nullptr, int iStartValue = 0);

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
  typename std::enable_if<!std::is_convertible<QString, T>::value, T>::type
  fromString(const QString& /*sVal*/, bool& bConversionStatus);

  template<typename T>
  typename std::enable_if<std::is_convertible<QString, T>::value, T>::type
  fromString(const QString& sVal, bool& bConversionStatus)
  {
    bConversionStatus = true;
    return T(sVal);
  }

  //-- QDateTime
  template<> QDateTime fromString<QDateTime>(const QString& sVal, bool& bConversionStatus);
  QString toString(const QDateTime& dt);

  //-- bool
  template<> bool fromString<bool>(const QString& sVal, bool& bConversionStatus);


  //-- std::vector<QUrl>
  template<> std::vector<QUrl> fromString<std::vector<QUrl>>(const QString& sVal, bool& bConversionStatus);
  QString toString(const std::vector<QUrl>& vUrls);
}

#endif // CONVERSION_H
