#ifndef PARAMETRIZABLEINTERFACE_H
#define PARAMETRIZABLEINTERFACE_H

#include "parameter.h"

#include <QString>
#include <QVariant>

#include <map>

class IParametrizable
{
public:
  virtual std::map<QString, SParameter> parameters() const = 0;

  virtual QVariant parameter(const QString& sName) const = 0;

  virtual bool setParameter(const QString& sName, const QVariant& value) = 0;

protected:
  IParametrizable() {}
  ~IParametrizable() {}
};

#endif // PARAMETRIZABLEINTERFACE_H
