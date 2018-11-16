#ifndef SERIALIZATIONENUMS_H
#define SERIALIZATIONENUMS_H

enum class ESerializingError
{
  eOk,
  eWrongParameter,
  eResourceError,
  eInternalError
};

enum class EDeserializingError
{
  eOk,
  eWrongParameter,
  eResourceError,
  eInternalError
};

#endif // SERIALIZATIONENUMS_H
