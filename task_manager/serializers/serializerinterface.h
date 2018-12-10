#ifndef SERIALIZER_INTERFACE_H
#define SERIALIZER_INTERFACE_H

#include "serializationenums.h"

#include <QString>
#include <QVariant>

#include <map>
#include <memory>

class Task;
class Group;
class SerializableManager;
class PropertyDescriptor;

class ISerializer
{
public:
  struct SParameter
  {
    QVariant value;
    bool bRequired;
  };

  virtual std::map<QString, SParameter> parameters() const = 0;

  virtual QVariant parameter(const QString& sName) const = 0;

  virtual bool setParameter(const QString& sName, const QVariant& value) = 0;

  virtual ESerializingError initSerialization() = 0;
  virtual ESerializingError deinitSerialization() = 0;

  virtual EDeserializingError initDeserialization() = 0;
  virtual EDeserializingError deinitDeserialization() = 0;

  virtual ESerializingError serialize(const SerializableManager&) = 0;

  virtual EDeserializingError deserialize(SerializableManager&) = 0;

  virtual ESerializingError serialize(const PropertyDescriptor&) = 0;

  virtual EDeserializingError deserialize(PropertyDescriptor&) = 0;

  virtual ESerializingError serialize(const Task&) = 0;

  virtual EDeserializingError deserialize(Task&) = 0;

  virtual ESerializingError serialize(const Group&) = 0;

  virtual EDeserializingError deserialize(Group&) = 0;

protected:
  ISerializer() {}

  virtual ~ISerializer() {}

  friend class SerializerFactoryPrivate;
};
typedef std::shared_ptr<ISerializer> tspSerializer;

#endif // SERIALIZER_INTERFACE_H
