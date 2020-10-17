#ifndef CONVERSION_H
#define CONVERSION_H

#include "libtaskmanager.h"

#include "conversion_fancy.h"

#include <type_traits>

#include <vector>

#include <QString>
#include <QDateTime>
#include <QUrl>
#include <QColor>

namespace conversion {

  template<typename T> QString toString(const T&);
  template<typename T> T fromString(const QString&, bool&);
}

namespace detail
{
  QStringList toList(const QString& s, const QChar& startChar, const QChar& endChar);


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


  // struct to allow partial template specialization
  // so we can implement converters for template classes
  template<typename T, typename T2 = void>
  struct to_string;

  template<typename T>
  struct to_string<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
    static QString convert(const T& num) {
      return QString::number(num);
    }
  };

  template<typename T>
  struct to_string<T, typename std::enable_if<std::is_convertible<T, int>::value &&
      !std::is_arithmetic<T>::value &&
      !std::is_convertible<T, QString>::value>::type> {
    static QString convert(const T& id) {
      return QString::number((int)id);
    }
  };

  template<typename T>
  struct to_string<T, typename std::enable_if<std::is_convertible<T, QString>::value>::type> {
    static QString convert(const T& val) {
      return QString(val);
    }
  };

  template<typename T>
  struct to_string<T, typename std::enable_if<detail::has_toString<T>::value>::type> {
    static QString convert(const T& t) {
      return t.toString();
    }
  };

  template<typename T>
  struct to_string<T, typename std::enable_if<detail::has_begin<T>::value &&
      detail::has_end<T>::value &&
      !std::is_same<T, QString>::value>::type> {
    static QString convert(const T& t) {
      QStringList list;
      for (const auto& el : t)
      {
        list.append(QString("{%1}").arg(conversion::toString(el)));
      }

      return list.join(",");
    }
  };


  template<typename T, typename T2 = void>
  struct from_string;

  template<typename T>
  struct from_string<T, typename std::enable_if<std::is_constructible<T, double>::value &&
      !std::is_constructible<T, QString>::value &&
      !std::is_constructible<T, std::string>::value &&
      !detail::has_push_back<T>::value>::type> {
    static T convert(const QString& s, bool& bConversionStatus) {
      // try to convert to double first
      double d = s.toDouble(&bConversionStatus);
      if (bConversionStatus) {
        return T(d);
      }

      // if that didn't work, try to convert 'fancy' value to int
      int i = conversion::fancy::toInt(s, &bConversionStatus);
      if (bConversionStatus) {
        return T(i);
      }

      return T();
    }
  };

  template<typename T>
  struct from_string<T, typename std::enable_if<std::is_constructible<T, QString>::value>::type> {
    static T convert(const QString& sVal, bool& bConversionStatus)
    {
      bConversionStatus = true;
      return T(sVal);
    }
  };

  //-- containers
  template<typename T>
  struct from_string<T, typename std::enable_if<detail::has_push_back<T>::value &&
      !std::is_same<T, QString>::value>::type> {
    static T convert(const QString& sVal, bool& bConversionStatus)
    {
      T container;

      // extract all top level elements {}
      QStringList list;
      if (sVal.contains("{") && sVal.contains("}")) {
        list = toList(sVal, '{', '}');
      }
      else {
        // legacy mode
        list = sVal.split("|");
      }
      for (const auto& el : list)
      {
        if (!el.isEmpty())
        {
          container.push_back(conversion::fromString<typename T::value_type>(el, bConversionStatus));
          if (!bConversionStatus)  { return T(); }
        }
      }

      bConversionStatus = true;

      return container;
    }
  };
}

namespace conversion
{

  template<typename T> QString toDisplayString(const T& val);


  template<typename T> QString toString(const T& t)
  {
    return detail::to_string<T>::convert(t);
  }

  template<typename T> T fromString(const QString& sVal, bool& bConversionStatus)
  {
    return detail::from_string<T>::convert(sVal, bConversionStatus);
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



  //-- QColor
  template<> QColor LIBTASKMANAGER fromString<QColor>(const QString& sVal, bool& bConversionStatus);
  QString LIBTASKMANAGER toString(const QColor& c);
}

#endif // CONVERSION_H
