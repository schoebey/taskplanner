#ifndef CONVERSION_H
#define CONVERSION_H

#include "libtaskmanager.h"

#include <type_traits>

#include <vector>

#include <QString>
#include <QDateTime>
#include <QUrl>
#include <QColor>

namespace detail
{
  template<typename T>
  using void_t = void;

template<typename T, typename = void>
struct has_push_back : std::false_type {};

template<typename T>
struct has_push_back<T,
    void_t<decltype(std::declval<T>().push_back(typename T::value_type{}))>>
                     : std::true_type{ };

template<typename T, typename = void>
struct has_begin : std::false_type {};

template<typename T>
struct has_begin<T,
    void_t<decltype(std::declval<T>().begin())>>
                     : std::true_type{ };

template<typename T, typename = void>
struct has_end : std::false_type {};

template<typename T>
struct has_end<T,
    void_t<decltype(std::declval<T>().end())>>
                     : std::true_type{ };

template<typename T, typename = void>
struct has_toString : std::false_type {};

template<typename T>
struct has_toString<T,
    void_t<decltype(std::declval<T>().toString())>>
                    : std::true_type {};


template<typename T, typename = void>
struct can_convert_to_string : std::false_type {};

template<typename T>
struct can_convert_to_string<T,
    void_t<typename std::enable_if<has_toString<T>::value ||
                        std::is_convertible<T, QString>::value, void>::type>>

 : std::true_type {};
}

namespace conversion
{
  template<typename T> QString toDisplayString(const T& val);

  static const QString c_sTimeFormat = "hh:mm:ss.zzz";
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
                          !std::is_convertible<T, int>::value &&
                          !detail::has_toString<T>::value &&
                          !std::is_arithmetic<T>::value &&
                          !detail::has_begin<T>::value &&
                          !detail::has_end<T>::value, QString>::type
  toString(const T& /*value*/);

  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, QString>::type
  toString(const T& num)
  {
    return QString::number(num);
  }

  template<typename T>
  typename std::enable_if<std::is_convertible<T, int>::value &&
                          !std::is_arithmetic<T>::value &&
                          !std::is_convertible<T, QString>::value, QString>::type
  toString(const T& id)
  {
    return QString::number((int)id);
  }

  template<typename T>
  typename std::enable_if<std::is_convertible<T, QString>::value, QString>::type
  toString(const T& val)
  {
    return QString(val);
  }

  template<typename T>
  typename std::enable_if<detail::has_toString<T>::value, QString>::type
  toString(const T& t)
  {
    return t.toString();
  }


  template<typename T>
  typename std::enable_if<!std::is_constructible<T, double>::value &&
                          !std::is_constructible<T, QString>::value &&
                          !detail::has_push_back<T>::value &&
                          !std::is_same<T, QString>::value, T>::type
  fromString(const QString& /*sVal*/, bool& /*bConversionStatus*/);

  template<typename T>
  typename std::enable_if<std::is_constructible<T, double>::value &&
                          !std::is_constructible<T, QString>::value &&
                          !detail::has_push_back<T>::value, T>::type
  fromString(const QString& s, bool& bConversionStatus)
  {
    // try to convert to double first
    double d = s.toDouble(&bConversionStatus);
    if (bConversionStatus) {
      return T(d);
    }

    // if that didn't work, try to convert 'fancy' value to int
    int i = fancy::toInt(s, &bConversionStatus);
    if (bConversionStatus) {
      return T(i);
    }

    return T();
  }

  template<typename T>
  typename std::enable_if<std::is_constructible<T, QString>::value, T>::type
  fromString(const QString& sVal, bool& bConversionStatus)
  {
    bConversionStatus = true;
    return T(sVal);
  }

  //-- QTime
  QTime LIBTASKMANAGER timeFromString(const QString& sVal, bool& bConversionStatus, const QTime& baseTime);
  template<> QTime LIBTASKMANAGER fromString<QTime>(const QString& sVal, bool& bConversionStatus);
  QString LIBTASKMANAGER toString(const QTime& dt);

  //-- QDateTime
  QDateTime LIBTASKMANAGER dateTimeFromString(const QString& sVal, bool& bConversionStatus, const QDateTime& baseDateTime);
  template<> QDateTime LIBTASKMANAGER fromString<QDateTime>(const QString& sVal, bool& bConversionStatus);
  QString LIBTASKMANAGER toString(const QDateTime& dt);

  //-- bool
  template<> bool LIBTASKMANAGER fromString<bool>(const QString& sVal, bool& bConversionStatus);
  QString LIBTASKMANAGER toString(bool bVal);

  //-- containers
  template<typename T>
  typename std::enable_if<detail::has_push_back<T>::value &&
                          !std::is_same<T, QString>::value, T>::type
  fromString(const QString& sVal, bool& bConversionStatus)
  {
    T container;

    auto list = sVal.split("|");
    for (const auto& el : list)
    {
      if (!el.isEmpty())
      {
        container.push_back(fromString<typename T::value_type>(el, bConversionStatus));
        if (!bConversionStatus)  { return T(); }
      }
    }

    bConversionStatus = true;

    return container;
  }


  template<typename T>
  typename std::enable_if<detail::has_begin<T>::value &&
                          detail::has_end<T>::value &&
                          !std::is_same<T, QString>::value, QString>::type
  toString(const T& t)
  {
    QStringList list;
    for (const auto& el : t)
    {
      list.append(toString(el));
    }

    return list.join("|");
  }

  //-- QColor
  template<> QColor LIBTASKMANAGER fromString<QColor>(const QString& sVal, bool& bConversionStatus);
  QString LIBTASKMANAGER toString(const QColor& c);
}

#endif // CONVERSION_H
