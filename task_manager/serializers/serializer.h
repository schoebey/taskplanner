#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "serializerinterface.h"

class Serializer : public ISerializer
{
public:
  Serializer();
  virtual ~Serializer();

  void registerParameter(const QString& sName, QVariant::Type type, bool bRequired);

  bool hasParameter(const QString& sName) const;

  std::map<QString, ISerializer::SParameter> parameters() const override;

  QVariant parameter(const QString& sName) const override;

  bool setParameter(const QString& sName, const QVariant& value) override;

protected:
  std::map<QString, SParameter> m_parameters;
};

#endif // SERIALIZER_H
