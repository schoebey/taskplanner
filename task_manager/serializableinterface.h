#ifndef SERIALIZABLEINTERFACE_H
#define SERIALIZABLEINTERFACE_H

#include "serializationenums.h"

class ISerializer;

class ISerializable
{
public:
  virtual int version() const = 0;

  virtual ESerializingError serialize(ISerializer* pSerializer) const = 0;

  virtual EDeserializingError deserialize(ISerializer* pSerializer) = 0;

protected:
  ISerializable() {}
  ~ISerializable() {}
};

#endif // SERIALIZABLEINTERFACE_H
