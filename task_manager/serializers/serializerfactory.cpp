#include "serializerfactory.h"
#include "serializerfactoryprivate.h"

SerializerFactoryPrivate::SerializerFactoryPrivate()
{
}

bool SerializerFactoryPrivate::registerCreator(tFnCreate fnCreate,
                                               const QString& sName,
                                               const QString& sExtension)
{
  SInternalInfo info(sName, sExtension, fnCreate);
  if (creators.find(sName) != creators.end())  { return false; }
  creators[sName] = info;

  return true;
}

std::vector<SSerializerInfo> SerializerFactoryPrivate::availableSerializers()
{
  std::vector<SSerializerInfo> vInfo;
  for (const auto& info : creators)
  {
    vInfo.push_back(info.second.info);
  }

  return vInfo;
}

tspSerializer SerializerFactoryPrivate::create(const QString& sName)
{
  // look up creator using sName
  auto it = creators.find(sName);
  if (it != creators.end())
  {
    tspSerializer spSerializer =
        std::shared_ptr<ISerializer>(it->second.fnCreator(), SerializerFactoryPrivate::destroy);

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

std::vector<SSerializerInfo> SerializerFactory::availableSerializers()
{
  return p()->availableSerializers();
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

