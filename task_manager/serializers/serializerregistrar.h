#ifndef SERIALIZERREGISTRATOR_H
#define SERIALIZERREGISTRATOR_H

#include "serializerfactory.h"

#include <functional>

class SerializerRegistrar : public SerializerFactory
{
public:
  SerializerRegistrar();

  static bool registerCreator(tFnCreate fnCreate, const QString& sName);
};

#endif // SERIALIZERREGISTRATOR_H
