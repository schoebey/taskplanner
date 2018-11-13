#ifndef TEXTSERIALIZER_H
#define TEXTSERIALIZER_H

#include "serializer.h"

class TextSerializer : public Serializer
{
public:
  TextSerializer();

  ESerializingError serialize(const Manager&) const override;

  EDeserializingError deserialize(Manager&) const override;
};

#endif // TEXTSERIALIZER_H
