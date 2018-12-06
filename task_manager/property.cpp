#include "property.h"


std::set<tspDescriptor> Properties::properties;
std::map<QString, std::function<tspProperty(const tspDescriptor&)>> Properties::factory;

namespace conversion
{
  template<> int fromString<int>(const QString& sVal, bool& bConversionStatus)
  {
    bConversionStatus = false;
    int iVal = sVal.toInt(&bConversionStatus);
    if (bConversionStatus)  { return iVal; }
    return 0;
  }

  static const QString c_sDateTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";
  static const std::set<QString> c_sDateTimeFormats = {c_sDateTimeFormat,
                                                       "yyyy-MM-dd hh:mm:ss",
                                                       "yyyy-MM-dd hh:mm",
                                                       "yyyy-MM-dd",
                                                       "yy-MM-dd",
                                                       "yyyy MM dd",
                                                       "yy MM dd",
                                                       "dd.MM.yyyy",
                                                       "dd.MM.yy"};
  template<> QDateTime fromString<QDateTime>(const QString& sVal, bool& bConversionStatus)
  {
    for (const auto& format : c_sDateTimeFormats)
    {
      QDateTime dt = QDateTime::fromString(sVal, format);
      bConversionStatus = dt.isValid();
      if (bConversionStatus)  { return dt; }
    }
    return QDateTime();
  }

  QString toString(const QDateTime& dt)
  {
    return dt.toString(c_sDateTimeFormat);
  }

  template<>
  bool fromString<bool>(const QString& sVal, bool& bConversionStatus)
  {
    bool bRv = 1 == sVal.toInt(&bConversionStatus);
    if (!bConversionStatus)
    {
      bConversionStatus = 0 == sVal.compare("true", Qt::CaseInsensitive) ||
                          0 == sVal.compare("false", Qt::CaseInsensitive);
      return 0 == sVal.compare("true", Qt::CaseInsensitive);
    }

    return bRv;
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
        bool bRv = spProp->setValue(sValue);
        return bRv && vals.insert(spProp).second;
      }
    }

    return false;
  }
  else
  {
    return (*it)->setValue(sValue);
  }

  return true;
}

QString Properties::get(const QString& sPropertyName) const
{
  auto it = std::find_if(vals.begin(), vals.end(),
                         [sPropertyName](const tspProperty& p)
  { return sPropertyName == p->descriptor()->name(); });
  if (it != vals.end())
  {
    return (*it)->value();
  }

  return QString();
}
