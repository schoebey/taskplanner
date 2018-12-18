#include "parametrizable.h"

Parametrizable::Parametrizable()
{
}

Parametrizable::~Parametrizable()
{
}

void Parametrizable::registerParameter(const QString& sName, QVariant::Type type, bool bRequired)
{
  SParameter param;
  param.value = QVariant(type);
  param.bRequired = bRequired;
  m_parameters[sName] = param;
}

bool Parametrizable::hasParameter(const QString& sName) const
{
  auto it = m_parameters.find(sName);
  if (m_parameters.end() != it)
  {
    return !it->second.value.isNull() &&
        it->second.value.isValid();
  }

  return false;
}

std::map<QString, SParameter> Parametrizable::parameters() const
{
  return m_parameters;
}

QVariant Parametrizable::parameter(const QString& sName) const
{
  auto it = m_parameters.find(sName);
  if (it != m_parameters.end())
  {
    return it->second.value;
  }

  return QVariant();
}

bool Parametrizable::setParameter(const QString& sName, const QVariant& value)
{
  auto it = m_parameters.find(sName);
  if (it != m_parameters.end() &&
      it->second.value.type() == value.type())
  {
    it->second.value = value;
    return true;
  }

  return false;
}
