#ifndef PROPERTY_H
#define PROPERTY_H

#include "constraint.h"
#include "conversion.h"
#include "serializableinterface.h"
#include "serializerinterface.h"
#include "libtaskmanager.h"

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

  virtual QString defaultValue() const = 0;

  virtual bool visible() const = 0;

  virtual tspConstraint constraint() const = 0;

  virtual void addConstraint(const QString& sName,
                             const QString& sConfiguration) = 0;
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
    // TODO: call all static register functions for all known constraints
    AndConstraint<T>::registerCreator();
    OrConstraint<T>::registerCreator();
    EqualsConstraint<T>::registerCreator();
  }

  int version() const override
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

  void setDefault(const T& t)
  {
    m_defaultFct = [t](){ return t; };
  }

  void setDefault(const std::function<T(void)> fnDefault)
  {
    m_defaultFct = fnDefault;
  }

  QString defaultValue() const override
  {
    if (hasDefault())
    {
      return conversion::toString(defaultValueT());
    }

    return QString();
  }

  bool hasDefault() const
  {
    return static_cast<bool>(m_defaultFct);
  }

  T defaultValueT() const
  {
    return m_defaultFct ? m_defaultFct() : T();
  }

  void setConstraint(const tspConstraintTpl<T>& spConstraint)
  {
    m_spConstraint = spConstraint;
  }

  tspConstraint constraint() const override
  {
    return m_spConstraint;
  }

  void addConstraint(const QString& sName,
                     const QString& sConfiguration) override
  {
    m_spConstraint = ConstraintFactory::create<T>(sName, sConfiguration);
  }

  bool accepts(const T& value) const
  {
    return nullptr == m_spConstraint || m_spConstraint->accepts(value);
  }

private:
  QString m_sName;
  QString m_sTypeName;
  bool m_bVisible = false;
  std::function<T(void)> m_defaultFct;
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
    return bRv ? set(val) : bRv;
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



namespace factory
{
  typedef std::map<QString, std::function<tspProperty(void)>> tProperties;
  typedef std::set<tspDescriptor> tDescriptors;

  LIBTASKMANAGER tProperties& propertiesFactory(size_t scopeHashCode, size_t typeHashCode = 0);
  LIBTASKMANAGER tDescriptors& descriptorsFactory(size_t scopeHashCode, size_t typeHashCode = 0);
}

#define REGISTER_PROPERTY(scope, name, type, visible) \
  Properties<scope>::registerProperty<type>(name, #type, visible)

/*
 the user has to be able to register new properties and assign them
 both a display widget and an editor.
 since the editor can be of unknown complexity, several functions
 should simplify their usage:
  * createEditor(T): creates and opens the editor and passes the current property to it (if necessary). Returns a widget pointer.
  * the editor needs a signal named accepted(T) that is emitted whenever the user enters a value the editor accepts.
    * the editor is closed after emitting this signal
  * the editor also needs a signal 'reject'. The editor is closed after sending this signal and the old value is retained.
  *
  the appropriate signals could be provided by a 'moderator' class that positions itself between the editor
  and the property storage


  what if the display widget takes care of editing as well?
  the widget would only need to have a setter and getter function for the property values,
  one to update its internal value with the current value of the property, one to update
  the current property value with the value the user has chosen.
  -> class editor
     +setPropertyValue(T) (public)
     +propertyChanged(QString) (signal), will be connected directly to the signal of the task widget

     the editor widget chooses to switch between editing and display view (depending on the editor,
     a switch might not even be necessary if the widget is in persistent editing mode (e.g. a checkbox)


   binding the editor to the property is bad,
   rather create an editor registry where an editor is bound to a property:
   EditorRegistry::registerEditor<property, editor>();
   EditorRegistry::createEditor<Property>();
*/
#define REGISTER_PROPERTY_EDITOR(scope, name, type, visible, editorClass) \

template<typename SCOPE>
class Properties
{
public:
  template <typename T> static tspDescriptorTpl<T> registerProperty(const QString& sName, const QString& sTypeName, bool bVisible)
  {
    tspDescriptorTpl<T> spDescriptor = std::make_shared<PropertyDescriptorTpl<T>>(sName, sTypeName, bVisible);
    if (nullptr == spDescriptor)
    {
      // typename was not found in known types - must be a custom typename
      assert(false && "unknown type");
      return nullptr;
    }

    // register the creator function for the new property
    std::function<tspProperty(void)> creatorFct =
        [spDescriptor]()
    {
      return std::make_shared<PropertyTpl<T>>(spDescriptor);
    };
    factory()[sName] = creatorFct;

    if (allDescriptors().insert(spDescriptor).second) {
      return spDescriptor;
    }

    return nullptr;
  }

  template<typename T> static bool registerConstraint(const QString& sName,
                                                      const tspConstraintTpl<T>& spConstraint)
  {
    tspDescriptor spDescriptor = findDescriptor(sName, allDescriptors());
    tspDescriptorTpl<T> spTypedDescriptor = std::static_pointer_cast<PropertyDescriptorTpl<T>>(spDescriptor);
    if (nullptr != spTypedDescriptor)
    {
      spTypedDescriptor->setConstraint(spConstraint);
      return true;
    }
    else
    {
      return false;
    }
  }

  static std::set<QString> registeredPropertyNames()
  {
    std::set<QString> names;
    for (const auto& el : allDescriptors())
    {
      names.insert(el->name());
    }
    return names;
  }

  std::set<QString> availablePropertyNames() const
  {
    std::set<QString> names;
    for (const auto& el : allProperties)
    {
      names.insert(el->descriptor()->name());
    }
    return names;
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
    return findDescriptor(sName, allDescriptors());
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
    tspProperty spProperty = findProperty(sPropertyName, allProperties);
    tspPropertyTpl<T> spTypedProperty = std::static_pointer_cast<PropertyTpl<T>>(spProperty);
    if (nullptr == spTypedProperty)
    {
      auto itCreator = factory().find(sPropertyName);
      if (itCreator != factory().end())
      {
        tspDescriptor spDescriptor = findDescriptor(sPropertyName, allDescriptors());
        tspDescriptorTpl<T> spTypedDescriptor = std::static_pointer_cast<PropertyDescriptorTpl<T>>(spDescriptor);
        if (nullptr != spTypedDescriptor && spTypedDescriptor->accepts(value))
        {
          tspPropertyTpl<T> spProp =
              std::make_shared<PropertyTpl<T>>(spTypedDescriptor, value);
          return allProperties.insert(spProp).second;
        }
      }

      return false;
    }
    else
    {
      return spTypedProperty->set(value);
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
      return conversion::fromString<T>((*it)->value(), bUnused);
    }

    return T();
  }

  bool set(const QString& sPropertyName, const QString& sValue)
  {
    auto it = std::find_if(allProperties.begin(), allProperties.end(),
                           [sPropertyName](const tspProperty& spProp)
    { return sPropertyName == spProp->descriptor()->name();});
    if (it == allProperties.end())
    {
      auto itCreator = factory().find(sPropertyName);
      if (itCreator != factory().end())
      {
        tspProperty spProp = itCreator->second();
        bool bRv = spProp->setValue(sValue);
        return bRv && allProperties.insert(spProp).second;
      }

      return false;
    }
    else
    {
      return (*it)->setValue(sValue);
    }

    return true;
  }

  QString get(const QString& sPropertyName) const
  {
    auto it = std::find_if(allProperties.begin(), allProperties.end(),
                           [sPropertyName](const tspProperty& p)
    { return sPropertyName == p->descriptor()->name(); });
    if (it != allProperties.end())
    {
      return (*it)->value();
    }

    return QString();
  }

  bool remove(const QString& sPropertyName)
  {
    auto it = std::find_if(allProperties.begin(), allProperties.end(),
                           [sPropertyName](const tspProperty& p)
    { return sPropertyName == p->descriptor()->name(); });
    if (it != allProperties.end())
    {
      allProperties.erase(it);
      return true;
    }

    return false;
  }

  bool isValid(const QString& sPropertyName) const
  {
    auto it = std::find_if(allProperties.begin(), allProperties.end(),
                           [sPropertyName](const tspProperty& p)
    { return sPropertyName == p->descriptor()->name(); });

    return it != allProperties.end();
  }

private:
  static std::set<tspDescriptor>& allDescriptors()
  {
    auto& d = factory::descriptorsFactory(typeid(SCOPE).hash_code());
    return d;
  }

  static factory::tProperties& factory()
  {
    auto& f = factory::propertiesFactory(typeid(SCOPE).hash_code());

    return f;
  }

private:
  std::set<tspProperty> allProperties;  // values
};


#endif // PROPERTY_H
