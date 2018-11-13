#include "serializerfactory.h"
#include "serializerfactoryprivate.h"

SerializerFactoryPrivate::SerializerFactoryPrivate()
{
}

bool SerializerFactoryPrivate::registerCreator(tFnCreate fnCreate, const QString& sName)
{
  if (creators.find(sName) == creators.end())  { return false; }
  creators[sName] = fnCreate;

  return true;
}

tspSerializer SerializerFactoryPrivate::create(const QString& sName)
{
  // look up creator using sName
  auto it = creators.find(sName);
  if (it != creators.end())
  {
    tspSerializer spSerializer =
        std::shared_ptr<ISerializer>(it->second(), SerializerFactoryPrivate::destroy);

    return spSerializer;
  }

  return nullptr;
}

void SerializerFactoryPrivate::destroy(ISerializer* pSerializer)
{
  delete pSerializer;
}


SerializerFactory::SerializerFactory()
{
}

tspSerializer SerializerFactory::create(const QString& sName)
{
  return p()->create(sName);
}

SerializerFactoryPrivate* SerializerFactory::p()
{
  static SerializerFactoryPrivate priv;
  return &priv;
}

