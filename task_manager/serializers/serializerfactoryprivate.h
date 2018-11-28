#ifndef SERIALIZERFACTORYPRIVATE_H
#define SERIALIZERFACTORYPRIVATE_H

#include "serializerinterface.h"
#include "serializerfactory.h"

#include <map>

class SerializerFactoryPrivate
{
public:
  SerializerFactoryPrivate();
  bool registerCreator(tFnCreate fnCreate, const QString& sName, const QString& sExtension);

  tspSerializer create(const QString& sName);

  static void destroy(ISerializer* pSerializer);

  std::vector<SSerializerInfo> availableSerializers();

private:
  std::map<QString, SInternalInfo> creators;
};

#endif // SERIALIZERFACTORYPRIVATE_H
