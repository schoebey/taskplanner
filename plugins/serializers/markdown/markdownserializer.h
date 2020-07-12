#ifndef MARKDOWNSERIALIZER_H
#define MARKDOWNSERIALIZER_H

#include "serializerinterface.h"
#include "property.h"

#include <QFile>
#include <QTextStream>

class MarkdownSerializer : public ISerializer
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

  ESerializingError serialize(const IConstraint&) override;

  EDeserializingError deserialize(IConstraint&) override;

  ESerializingError serialize(const Task&) override;

  EDeserializingError deserialize(Task&) override;

  ESerializingError serialize(const Tag&) override;

  EDeserializingError deserialize(Tag&) override;

  ESerializingError serialize(const Group&) override;

  EDeserializingError deserialize(Group&) override;

private:
  QFile m_file;
  QTextStream* m_pStream = nullptr;
  int m_iFileVersion = 0;
};

#endif // MARKDOWNSERIALIZER_H
