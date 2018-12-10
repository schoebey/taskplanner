#ifndef TEXTSERIALIZER_H
#define TEXTSERIALIZER_H

#include "serializer.h"

#include <QFile>
#include <QTextStream>

class TextSerializer : public Serializer
{
public:
  TextSerializer();

  ESerializingError initSerialization() override;
  ESerializingError deinitSerialization() override;

  EDeserializingError initDeserialization() override;
  EDeserializingError deinitDeserialization() override;

  ESerializingError serialize(const SerializableManager&) override;

  EDeserializingError deserialize(SerializableManager&) override;

  ESerializingError serialize(const PropertyDescriptor&) override;

  EDeserializingError deserialize(PropertyDescriptor&) override;

  ESerializingError serialize(const IConstraint&) override;

  EDeserializingError deserialize(IConstraint&) override;

  ESerializingError serialize(const Task&) override;

  EDeserializingError deserialize(Task&) override;

  ESerializingError serialize(const Group&) override;

  EDeserializingError deserialize(Group&) override;

private:
  QFile m_file;
  QTextStream m_stream;
};

#endif // TEXTSERIALIZER_H
