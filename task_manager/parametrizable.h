#ifndef PARAMETRIZABLE_H
#define PARAMETRIZABLE_H

#include "parametrizableinterface.h"

class Parametrizable : public IParametrizable
{
public:
  Parametrizable();
  virtual ~Parametrizable();

  void registerParameter(const QString& sName, QVariant::Type type, bool bRequired);

  bool hasParameter(const QString& sName) const;

  std::map<QString, SParameter> parameters() const override;

  QVariant parameter(const QString& sName) const override;

  bool setParameter(const QString& sName, const QVariant& value) override;

protected:
  std::map<QString, SParameter> m_parameters;
};

#endif // PARAMETRIZABLE_H