#ifndef REGISTER_SERIALIZER_H
#define REGISTER_SERIALIZER_H

#include "patterns/factory/register.h"
#include "serializerinterface.h"

template<typename T> class RegisterSerializer : public Register<T, ISerializer, QString>
{
public:
  RegisterSerializer(const QString& sName, const QString& sInfo)
    : Register<T, ISerializer, QString>(sName, sInfo)
  {}
};

#endif // REGISTER_SERIALIZER_H
