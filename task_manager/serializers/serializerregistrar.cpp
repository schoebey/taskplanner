#include "serializerregistrar.h"
#include "serializerfactoryprivate.h"

SerializerRegistrar::SerializerRegistrar()
{
}

bool SerializerRegistrar::registerCreator(tFnCreate fnCreate,
                                          const QString& sName,
                                          const QString& sExtension)
{
  return p()->registerCreator(fnCreate, sName, sExtension);
}
