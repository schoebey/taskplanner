#include "serializer.h"
#include <QVariant>

Serializer::Serializer()
{

}

void Serializer::registerParameter(const QString& sName, QVariant::Type type, bool bRequired)
{
  ISerializer::SParameter param;
  param.value = QVariant(type);
  param.bRequired = bRequired;
  m_parameters[sName] = param;
}

bool Serializer::hasParameter(const QString& sName) const
{
  auto it = m_parameters.find(sName);
  if (m_parameters.end() != it)
  {
    return it->second.value.isValid();
  }

  return false;
}

std::map<QString, ISerializer::SParameter> Serializer::parameters() const
{
  return m_parameters;
}

QVariant Serializer::parameter(const QString& sName) const
{
  auto it = m_parameters.find(sName);
  if (it != m_parameters.end())
  {
    return it->second.value;
  }

  return QVariant();
}

bool Serializer::setParameter(const QString& sName, const QVariant& value)
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

