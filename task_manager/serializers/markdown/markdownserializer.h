#ifndef MARKDOWNSERIALIZER_H
#define MARKDOWNSERIALIZER_H

#include "serializer.h"
#include "property.h"

#include <QFile>
#include <QTextStream>

class MarkdownSerializer : public Serializer
{
public:
  MarkdownSerializer();

  ESerializingError initSerialization() override;
  ESerializingError deinitSerialization() override;

  EDeserializingError initDeserialization() override;
  EDeserializingError deinitDeserialization() override;

  ESerializingError serialize(const SerializableManager&) override;

  EDeserializingError deserialize(SerializableManager&) override;

  ESerializingError serialize(const PropertyDescriptor&) override;

  EDeserializingError deserialize(PropertyDescriptor&) override;

  ESerializingError serialize(const Task&) override;

  EDeserializingError deserialize(Task&) override;

  ESerializingError serialize(const Group&) override;

  EDeserializingError deserialize(Group&) override;

private:
  QFile m_file;
  QTextStream m_stream;
};

#endif // MARKDOWNSERIALIZER_H
