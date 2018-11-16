#ifndef SERIALIZERFACTORYPRIVATE_H
#define SERIALIZERFACTORYPRIVATE_H

#include "serializerinterface.h"
#include "serializerfactory.h"

#include <map>

class SerializerFactoryPrivate
{
public:
  SerializerFactoryPrivate();
  bool registerCreator(tFnCreate fnCreate, const QString& sName);

  tspSerializer create(const QString& sName);

  static void destroy(ISerializer* pSerializer);

private:
  std::map<QString, tFnCreate> creators;
};

#endif // SERIALIZERFACTORYPRIVATE_H
