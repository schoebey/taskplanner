#include "textserializer.h"
#include "register.h"
#include "manager.h"

#include <QString>
#include <QVariant>
#include <QFile>

namespace
{
  Register<TextSerializer> s("Text");

  static const QString c_sPara_FileName = "fileName";
}

TextSerializer::TextSerializer()
{
  registerParameter(c_sPara_FileName, QVariant::String, true);
}

ESerializingError TextSerializer::serialize(const Manager&) const
{
  if (hasParameter(c_sPara_FileName))
  {
    QString sFileName = parameter(c_sPara_FileName).toString();
    QFile f(sFileName);
    if (f.open(QIODevice::ReadWrite))
    {
      return ESerializingError::eOk;
    }

    return ESerializingError::eResourceError;
  }
  else
  {
    return ESerializingError::eWrongParameter;
  }
}

EDeserializingError TextSerializer::deserialize(Manager&) const
{
  if (hasParameter(c_sPara_FileName))
  {
    QString sFileName = parameter(c_sPara_FileName).toString();
    QFile f(sFileName);
    if (f.open(QIODevice::ReadOnly))
    {
      return EDeserializingError::eOk;
    }

    return EDeserializingError::eResourceError;
  }
  else
  {
    return EDeserializingError::eWrongParameter;
  }
}
