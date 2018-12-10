#ifndef CONVERSION_H
#define CONVERSION_H

#include <type_traits>

#include <QString>
#include <QDateTime>

namespace conversion
{
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
}

#endif // CONVERSION_H