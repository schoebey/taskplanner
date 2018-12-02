#include "property.h"


std::set<tspDescriptor> Properties::properties;
std::map<QString, std::function<tspProperty(const tspDescriptor&)>> Properties::factory;

namespace conversion
{
  template<> int fromString(const QString& sVal)
  {
    bool bOk = false;
    int iVal = sVal.toInt(&bOk);
    if (bOk)  { return iVal; }
    return 0;
  }

  static const QString c_sDateTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";
  template<> QDateTime fromString(const QString& sVal)
  {
    return QDateTime::fromString(sVal, c_sDateTimeFormat);
  }
  template<> QString toString(const QDateTime& dt)
  {
    return dt.toString(c_sDateTimeFormat);
  }
}

std::set<QString> Properties::registeredPropertyNames()
{
  std::set<QString> names;
  for (const auto& el : properties)
  {
    names.insert(el->name());
  }
  return names;
}

std::set<QString> Properties::availablePropertyNames() const
{
  std::set<QString> names;
  for (const auto& el : vals)
  {
    names.insert(el->descriptor()->name());
  }
  return names;
}

bool Properties::set(const QString& sPropertyName, const QString& sValue)
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
        tspProperty spProp = itCreator->second(spDescriptor);
        return vals.insert(spProp).second;
      }
    }

    return false;
  }
  else
  {
    (*it)->value()->setValue(sValue);
  }

  return true;
}

QString Properties::get(const QString& sPropertyName)
{
  auto it = std::find_if(vals.begin(), vals.end(),
                         [sPropertyName](const tspProperty& p)
  { return sPropertyName == p->descriptor()->name(); });
  if (it != vals.end())
  {
    return (*it)->value()->value();
  }

  return QString();
}
