#ifndef SERIALIZERFACTORY_H
#define SERIALIZERFACTORY_H

#include "serializerinterface.h"
#include "serializerinfo.h"

#include <QString>

#include <vector>
#include <functional>


class SerializerFactoryPrivate;

class SerializerFactory
{
public:
  SerializerFactory();

  static std::vector<SSerializerInfo> availableSerializers();

  static tspSerializer create(const QString& sName);

protected:
  static SerializerFactoryPrivate* p();
};

#endif // SERIALIZERFACTORY_H
