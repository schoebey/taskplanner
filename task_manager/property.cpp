#include "property.h"


std::set<tspDescriptor> Properties::properties;

namespace conversion
{
  template<> int fromString(const QString& sVal)
  {
    bool bOk = false;
    int iVal = sVal.toInt(&bOk);
    if (bOk)  { return iVal; }
    return 0;
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
