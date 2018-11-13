#ifndef SERIALIZER_INTERFACE_H
#define SERIALIZER_INTERFACE_H

#include <QString>
#include <QVariant>

#include <map>
#include <memory>

class Manager;

enum class ESerializingError
{
  eOk,
  eWrongParameter,
  eResourceError
};

enum class EDeserializingError
{
  eOk,
  eWrongParameter,
  eResourceError
};

class ISerializer
{
public:
  struct SParameter
  {
    QVariant value;
    bool bRequired;
  };


  virtual ESerializingError serialize(const Manager&) const = 0;

  virtual EDeserializingError deserialize(Manager&) const = 0;

  virtual std::map<QString, SParameter> parameters() const = 0;

  virtual QVariant parameter(const QString& sName) const = 0;

  virtual bool setParameter(const QString& sName, const QVariant& value) = 0;

protected:
  ISerializer() {}

  virtual ~ISerializer() {}

  friend class SerializerFactoryPrivate;
};
typedef std::shared_ptr<ISerializer> tspSerializer;

#endif // SERIALIZER_INTERFACE_H
