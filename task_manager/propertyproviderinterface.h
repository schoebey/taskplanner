#ifndef PROPERTYPROVIDERINTERFACE_H
#define PROPERTYPROVIDERINTERFACE_H

#include <QString>

#include <set>

class IPropertyProvider
{
public:
  virtual std::set<QString> propertyNames() const = 0;
  virtual bool hasPropertyValue(const QString& sName) const = 0;
  virtual QString propertyValue(const QString& sName) const = 0;
  virtual bool setPropertyValue(const QString& sName, const QString& sValue) = 0;

protected:
  IPropertyProvider() {}
  ~IPropertyProvider() {}
};

#endif // PROPERTYPROVIDERINTERFACE_H
