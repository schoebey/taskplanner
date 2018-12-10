#ifndef PROPERTY_H
#define PROPERTY_H

#include "constraint.h"
#include "conversion.h"
#include "serializableinterface.h"
#include "serializerinterface.h"

#include <QString>
#include <QDateTime>

#include <memory>
#include <map>
#include <set>
#include <cassert>
#include <iostream>
#include <functional>

class PropertyDescriptor : public ISerializable
{
public:
  PropertyDescriptor() {}
  virtual ~PropertyDescriptor() {}

  virtual QString name() const = 0;

  virtual QString typeName() const = 0;

  virtual bool visible() const = 0;

  virtual tspConstraint constraint() const = 0;
};
typedef std::shared_ptr<PropertyDescriptor> tspDescriptor;

template<typename T> class PropertyDescriptorTpl : public PropertyDescriptor
{
public:
  PropertyDescriptorTpl(const QString& sName,
                        const QString& sTypeName = QString(),
                        bool bVisible = true)
   : m_sName(sName),
     m_sTypeName(sTypeName),
     m_bVisible(bVisible)
  {
  }

  int version() const
  {
    return 0;
  }

  ESerializingError serialize(ISerializer* pSerializer) const override
  {
    return pSerializer->serialize(*this);
  }

  EDeserializingError deserialize(ISerializer* pSerializer) override
  {
    return pSerializer->deserialize(*this);
  }

  QString name() const override
  {
    return m_sName;
  }

  QString typeName() const override
  {
    return m_sTypeName.isEmpty() ? typeid(T).name() : m_sTypeName;
  }

  bool visible() const override
  {
    return m_bVisible;
  }

  void setConstraint(const tspConstraintTpl<T>& spConstraint)
  {
    m_spConstraint = spConstraint;
  }

  tspConstraint constraint() const override
  {
    return m_spConstraint;
  }

  bool accepts(const T& value) const
  {
    return nullptr == m_spConstraint || m_spConstraint->accepts(value);
  }

private:
  QString m_sName;
  QString m_sTypeName;
  bool m_bVisible = false;
  tspConstraintTpl<T> m_spConstraint;
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

  T get() const
  {
    return m_value;
  }

  void set(const T& t)
  {
    m_value = t;
  }

  QString value() const override
  {
    return conversion::toString(m_value);
  }

  bool setValue(const QString& sVal) override
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

  virtual QString value() const = 0;
  virtual bool setValue(const QString& sVal) = 0;

private:
};
typedef std::shared_ptr<Property> tspProperty;

template <typename T> class PropertyTpl : public Property
{
public:
  PropertyTpl(const tspDescriptorTpl<T>& spDescriptor, const T& value = T())
    : m_spDescriptor(spDescriptor),
      m_spValue(new PropertyValueTpl<T>(value))
  {}

  tspDescriptor descriptor() const override
  {
    return m_spDescriptor;
  }

  QString value() const override
  {
    return m_spValue->value();
  }

  bool setValue(const QString& sVal) override
  {
    bool bRv = false;
    T val = conversion::fromString<T>(sVal, bRv);
    return set(val);
  }

  T get() const
  {
    return m_spValue->get();
  }

  bool set(const T& val)
  {
    if (nullptr != m_spDescriptor && m_spDescriptor->accepts(val))
    {
      m_spValue->set(val);
      return true;
    }

    return false;
  }

private:
  tspDescriptorTpl<T> m_spDescriptor;
  tspValueTpl<T> m_spValue;
};

template<typename T> using tspPropertyTpl = std::shared_ptr<PropertyTpl<T>>;



#define REGISTER_PROPERTY(name, type, visible) Properties::registerProperty<type>(name, #type, visible);

class Properties
{
public:
  template <typename T> static bool registerProperty(const QString& sName, const QString& sTypeName, bool bVisible)
  {
    tspDescriptorTpl<T> spDescriptor = std::make_shared<PropertyDescriptorTpl<T>>(sName, sTypeName, bVisible);
    if (nullptr == spDescriptor)
    {
      // typename was not found in known types - must be a custom typename
      assert(false && "unknown type");
      return false;
    }

    // register the creator function for the new property
    std::function<tspProperty(void)> creatorFct =
        [spDescriptor]()
    {
      return std::make_shared<PropertyTpl<T>>(spDescriptor);
    };
    factory[sName] = creatorFct;

    descriptors<T>().insert(spDescriptor);
    return allDescriptors.insert(spDescriptor).second;
  }

  template<typename T> static bool registerConstraint(const QString& sName,
                                                      const tspConstraintTpl<T>& spConstraint)
  {
    tspDescriptorTpl<T> spDescriptor = findDescriptor(sName, descriptors<T>());
    if (nullptr != spDescriptor)
    {
      spDescriptor->setConstraint(spConstraint);
      return true;
    }
    else
    {
      return false;
    }
  }

  static std::set<QString> registeredPropertyNames();

  std::set<QString> availablePropertyNames() const;

  template<typename T> static std::set<tspDescriptorTpl<T>>& descriptors()
  {
    static std::set<tspDescriptorTpl<T>> descriptors;
    return descriptors;
  }

  template<typename T>
  static T findDescriptor(const QString& sName,
                          const std::set<T>& container)
  {
    auto it = std::find_if(container.begin(),
                           container.end(),
                           [sName](const T& spDescriptor)
    {
      return nullptr != spDescriptor &&
          sName == spDescriptor->name();
    });

    if (it != container.end())  { return *it; }

    return nullptr;
  }

  template<typename T> static std::set<tspPropertyTpl<T>>& properties(const Properties* pOwner)
  {
    static std::map<const Properties*, std::set<tspPropertyTpl<T>>> properties;
    return properties[pOwner];
  }

  template<typename T>
  static T findProperty(const QString& sName,
                        const std::set<T>& set)
  {
    auto it = std::find_if(set.begin(),
                           set.end(),
                           [sName](const T& spProperty)
    {
      return nullptr != spProperty &&
          sName == spProperty->descriptor()->name();
    });

    if (it != set.end())  { return *it; }

    return nullptr;
  }

  static tspDescriptor descriptor(const QString& sName)
  {
    return findDescriptor(sName, allDescriptors);
  }

  static bool visible(const QString& sPropertyName)
  {
    tspDescriptor spDescriptor = descriptor(sPropertyName);
    if (nullptr != spDescriptor)
    {
      return spDescriptor->visible();
    }

    return false;
  }

  template<typename T> bool set(const QString& sPropertyName, const T& value)
  {
    tspPropertyTpl<T> spProperty = findProperty(sPropertyName, properties<T>(this));

    if (nullptr == spProperty)
    {
      auto itCreator = factory.find(sPropertyName);
      if (itCreator != factory.end())
      {
        tspDescriptorTpl<T> spDescriptor = findDescriptor(sPropertyName, descriptors<T>());
        if (nullptr != spDescriptor && spDescriptor->accepts(value))
        {
          tspPropertyTpl<T> spProp =
              std::make_shared<PropertyTpl<T>>(spDescriptor, value);
          properties<T>(this).insert(spProp);
          return allProperties.insert(spProp).second;
        }
      }

      return false;
    }
    else
    {
      return spProperty->set(value);
    }
  }

  template<typename T> T get(const QString& sPropertyName) const
  {
    auto it = std::find_if(allProperties.begin(), allProperties.end(),
                           [sPropertyName](const tspProperty& p)
    { return sPropertyName == p->descriptor()->name(); });
    if (it != allProperties.end())
    {
      bool bUnused = false;
      conversion::fromString<T>((*it)->value(), bUnused);
    }

    return T();
  }

  bool set(const QString& sPropertyName, const QString& sValue);
  QString get(const QString& sPropertyName) const;

private:
  static std::set<tspDescriptor> allDescriptors; // descriptions, without values
  static std::map<QString, std::function<tspProperty(void)>> factory; // needed for creating properties of unspecified
  std::set<tspProperty> allProperties;  // values
};


#endif // PROPERTY_H
