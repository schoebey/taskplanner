#ifndef SERIALIZERFACTORY_H
#define SERIALIZERFACTORY_H

#include "serializerinterface.h"

#include <QString>

#include <vector>
#include <functional>

typedef std::function<ISerializer*(void)> tFnCreate;

class SerializerFactoryPrivate;

class SerializerFactory
{
public:
  SerializerFactory();

  static std::vector<QString> availableSerializers();

  static tspSerializer create(const QString& sName);

protected:
  static SerializerFactoryPrivate* p();
};

#endif // SERIALIZERFACTORY_H
