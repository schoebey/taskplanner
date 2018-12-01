#ifndef PROPERTY_H
#define PROPERTY_H

#include <QString>

#include <memory>
#include <map>
#include <set>
#include <cassert>





namespace conversion
{
  template<typename T> QString toString(const T& value);

  template<typename T> typename std::enable_if<std::is_arithmetic<T>::value, QString>::type
  toString(const T& num)
  {
    return QString::number(num);
  }

  template<typename T> typename std::enable_if<std::is_convertible<T, QString>::value, QString>::type
  toString(const T& val)
  {
    return QString(val);
  }


  template<typename T> T fromString(const QString& /*sVal*/)
  {
    static_assert(!std::is_arithmetic<T>::value, "implement for type");
  }

  template<> int fromString(const QString& sVal)
  {
    bool bOk = false;
    int iVal = sVal.toInt(&bOk);
    if (bOk)  { return iVal; }
    return 0;
  }
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

  virtual void setValue(const QString& sVal) = 0;
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

  void setValue(const QString& sVal)
  {
    m_value = conversion::fromString<T>(sVal);
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

  virtual PropertyValue* value() const = 0;

private:
};

template <typename T> class PropertyTpl : public Property
{
public:
  PropertyTpl(const tspDescriptorTpl<T>& spDescriptor, const T& value)
    : m_spDescriptor(spDescriptor),
      m_value(value)
  {}

  tspDescriptor descriptor() const override
  {
    return m_spDescriptor;
  }

  PropertyValue* value() const override
  {
    return &m_value;
  }

private:
  tspDescriptorTpl<T> m_spDescriptor;
  PropertyValueTpl<T> m_value;
};




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
    if (sTypeName == typeid(typename std::tuple_element<I, std::tuple<Tp...>>::type).name())
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




class Properties
{
public:
  static bool registerProperty(const QString& sName, const QString& sTypeName)
  {
    tspDescriptor spDescriptor = nullptr;

    // add known types to tuple for registration
    std::tuple<int, double, bool, QString> types;

    spDescriptor = detail::propertyFromType(types, sName, sTypeName);
    if (nullptr == spDescriptor)
    {
      // typename was not found in known types - must be a custom typename
      assert(false && "unknown type");
      return false;
    }

    return properties.insert(spDescriptor).second;
  }

  static bool registerProperty(const tspDescriptor& spDescriptor)
  {
    return properties.insert(spDescriptor).second;
  }

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
                           [sPropertyName](const Property& p)
    { return sPropertyName == p.descriptor()->name();});
    if (it == vals.end())
    {
      tspDescriptor spDescriptor = descriptor(sPropertyName);
      if (nullptr != spDescriptor)
      {
        PropertyTpl<T> prop(spDescriptor, value);
        vals.insert(prop);
      }
      else
      {
        return false;
      }
    }
    else
    {
      it->value()->setValue(conversion::toString(value));
    }

    return true;
  }

  template<typename T> T get(const QString& sPropertyName) const
  {
    auto it = std::find_if(vals.begin(), vals.end(),
                           [sPropertyName](const Property& p)
    { return sPropertyName == p.descriptor()->name(); });
    if (it != vals.end())
    {
      return conversion::fromString<T>(it->value()->value());
    }
  }

private:
  static std::set<tspDescriptor> properties; // descriptions, without values
  std::set<Property> vals;  // values
};


#endif // PROPERTY_H
