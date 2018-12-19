#ifndef SERIALIZER_INTERFACE_H
#define SERIALIZER_INTERFACE_H

#include "serializationenums.h"
#include "parametrizable.h"

#include <QString>
#include <QVariant>

#include <map>
#include <memory>

class Task;
class Group;
class SerializableManager;
class PropertyDescriptor;
class IConstraint;

class ISerializer : public Parametrizable
{
public:
  virtual ~ISerializer() {}

  virtual ESerializingError initSerialization() = 0;
  virtual ESerializingError deinitSerialization() = 0;

  virtual EDeserializingError initDeserialization() = 0;
  virtual EDeserializingError deinitDeserialization() = 0;

  virtual ESerializingError serialize(const SerializableManager&) = 0;

  virtual EDeserializingError deserialize(SerializableManager&) = 0;

  virtual ESerializingError serialize(const PropertyDescriptor&) = 0;

  virtual EDeserializingError deserialize(PropertyDescriptor&) = 0;

  virtual ESerializingError serialize(const IConstraint&) = 0;

  virtual EDeserializingError deserialize(IConstraint&) = 0;

  virtual ESerializingError serialize(const Task&) = 0;

  virtual EDeserializingError deserialize(Task&) = 0;

  virtual ESerializingError serialize(const Group&) = 0;

  virtual EDeserializingError deserialize(Group&) = 0;

protected:
  ISerializer() {}
};
typedef std::shared_ptr<ISerializer> tspSerializer;

#endif // SERIALIZER_INTERFACE_H
