#ifndef PROPERTY_H
#define PROPERTY_H

#include <QString>
#include <QDateTime>

#include <memory>
#include <map>
#include <set>
#include <cassert>
#include <iostream>



namespace conversion
{
  template<typename T>
  typename std::enable_if<!std::is_convertible<T, QString>::value, QString>::type
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
template<> QString toString(const QDateTime& dt);
}




class PropertyDescriptor
{
public:
  PropertyDescriptor() {}
  virtual ~PropertyDescriptor() {}

  virtual QString name() const = 0;

  virtual QString typeName() const = 0;
};
typedef std::shared_ptr<PropertyDescriptor> tspDescriptor;

template<typename T> class PropertyDescriptorTpl : public PropertyDescriptor
{
public:
  PropertyDescriptorTpl(const QString& sName, const QString& sTypeName = QString())
   : m_sName(sName),
     m_sTypeName(sTypeName)
  {
  }

  QString name() const override
  {
    return m_sName;
  }

  QString typeName() const override
  {
    return m_sTypeName.isEmpty() ? typeid(T).name() : m_sTypeName;
  }

private:
  QString m_sName;
  QString m_sTypeName;
};
template <typename T> using tspDescriptorTpl = std::shared_ptr<PropertyDescriptorTpl<T>>;


class PropertyValue
{
public:
  PropertyValue() {}
  virtual ~PropertyValue() {}

  virtual QString value() const = 0;

  virtual bool setValue(const QString& sVal) = 0;
};
typedef std::shared_ptr<PropertyValue> tspValue;

template<typename T> class PropertyValueTpl : public PropertyValue
{
public:
  PropertyValueTpl()
  {
  }

  PropertyValueTpl(const T& val)
    : m_value(val)
  {
  }

  operator T() const
  {
    return m_value;
  }

  void operator= (const T& t)
  {
    m_value = t;
  }

  QString value() const override
  {
    return conversion::toString<T>(m_value);
  }

  bool setValue(const QString& sVal)
  {
    bool bRv = false;
    T val = conversion::fromString<T>(sVal, bRv);
    if (bRv)  { m_value = val; }
    return bRv;
  }

private:
  T m_value;
};
template <typename T> using tspValueTpl = std::shared_ptr<PropertyValueTpl<T>>;



class Property
{
public:
  Property() {}
  virtual ~Property() {}

  virtual tspDescriptor descriptor() const = 0;

  virtual tspValue value() const = 0;

private:
};
typedef std::shared_ptr<Property> tspProperty;

template <typename T> class PropertyTpl : public Property
{
public:
  PropertyTpl(const tspDescriptor& spDescriptor, const T& value = T())
    : m_spDescriptor(spDescriptor),
      m_spValue(new PropertyValueTpl<T>(value))
  {}

  tspDescriptor descriptor() const override
  {
    return m_spDescriptor;
  }

  tspValue value() const override
  {
    return m_spValue;
  }

private:
  tspDescriptor m_spDescriptor;
  tspValue m_spValue;
};

template<typename T> using tspPropertyTpl = std::shared_ptr<PropertyTpl<T>>;


namespace detail
{
  template <int N, typename... Ts>
  struct get;

  template <int N, typename T, typename... Ts>
  struct get<N, std::tuple<T, Ts...>>
  {
      using type = typename get<N - 1, std::tuple<Ts...>>::type;
  };

  template <typename T, typename... Ts>
  struct get<0, std::tuple<T, Ts...>>
  {
      using type = T;
  };


  template<std::size_t I = 0, typename... Tp>
  inline typename std::enable_if<I == sizeof...(Tp), tspDescriptor>::type
  propertyFromType(std::tuple<Tp...>& /*t*/, const QString& /*sName*/, const QString& /*sTypeName*/)
  {
    return nullptr;
  }

  template<std::size_t I = 0, typename... Tp>
  inline typename std::enable_if<I < sizeof...(Tp), tspDescriptor>::type
  propertyFromType(std::tuple<Tp...>& t, const QString& sName, const QString& sTypeName)
  {
    std::cout << typeid(typename std::tuple_element<I, std::tuple<Tp...>>::type).name() << std::endl;
    QString sCurrentTypeName(typeid(typename std::tuple_element<I, std::tuple<Tp...>>::type).name());
    if (sTypeName == sCurrentTypeName)
    {
       return std::make_shared<PropertyDescriptorTpl<typename std::tuple_element<I, std::tuple<Tp...>>::type>>(sName);
    }


    return propertyFromType<I + 1, Tp...>(t, sName, sTypeName);
  }
}



//// ctor function
//typedef std::function<Property(name, data, typename)> fnCreate;
//class PropertyFactory
//{
//public:
////  static bool registerProperty(const QString& sName, const QString& sTypeName)
////  {

////  }

//private:
//  PropertyFactory() {}
//  ~PropertyFactory() {}

//  static std::map<QString, fnCreate> registry;
//};

//#define PROPERTY(type, name, value) PropertyTpl<type>(name, value, #type)


#define REGISTER_PROPERTY(name, type) Properties::registerProperty<type>(name, #type);

class Properties
{
public:
  template <typename T> static bool registerProperty(const QString& sName, const QString& sTypeName)
  {
    tspDescriptor spDescriptor = std::make_shared<PropertyDescriptorTpl<T>>(sName, sTypeName);
    if (nullptr == spDescriptor)
    {
      // typename was not found in known types - must be a custom typename
      assert(false && "unknown type");
      return false;
    }

    // register the creator function for the new property
    std::function<tspProperty(const tspDescriptor&)> creatorFct =
        [](const tspDescriptor& spDescriptor)
    {
      return std::make_shared<PropertyTpl<T>>(spDescriptor);
    };
    factory[sName] = creatorFct;

    return properties.insert(spDescriptor).second;
  }

  static std::set<QString> registeredPropertyNames();

  std::set<QString> availablePropertyNames() const;

  static tspDescriptor descriptor(const QString& sName)
  {
    auto it = std::find_if(properties.begin(),
                           properties.end(),
                           [sName](const tspDescriptor& spDescriptor)
    {
      return nullptr != spDescriptor &&
          sName == spDescriptor->name();
    });

    if (it != properties.end())  { return *it; }

    return nullptr;
  }

  template<typename T> bool set(const QString& sPropertyName, const T& value)
  {
    auto it = std::find_if(vals.begin(), vals.end(),
                           [sPropertyName](const tspProperty& spProp)
    { return sPropertyName == spProp->descriptor()->name();});
    if (it == vals.end())
    {
      tspDescriptor spDescriptor = descriptor(sPropertyName);
      if (nullptr != spDescriptor)
      {
        auto itCreator = factory.find(sPropertyName);
        if (itCreator != factory.end())
        {
          tspPropertyTpl<T> spProp =
              std::make_shared<PropertyTpl<T>>(spDescriptor, value);
          return vals.insert(spProp).second;
        }
      }

      return false;
    }
    else
    {
      (*it)->value()->setValue(conversion::toString(value));
    }

    return true;
  }

  template<typename T> T get(const QString& sPropertyName) const
  {
    auto it = std::find_if(vals.begin(), vals.end(),
                           [sPropertyName](const tspProperty& p)
    { return sPropertyName == p->descriptor()->name(); });
    if (it != vals.end())
    {
      bool bUnused = false;
      conversion::fromString<T>((*it)->value()->value(), bUnused);
    }

    return T();
  }

  bool set(const QString& sPropertyName, const QString& sValue);
  QString get(const QString& sPropertyName) const;

private:
  static std::set<tspDescriptor> properties; // descriptions, without values
  static std::map<QString, std::function<tspProperty(const tspDescriptor&)>> factory;
  std::set<tspProperty> vals;  // values
};


#endif // PROPERTY_H
