#ifndef SERIALIZERFACTORY_H
#define SERIALIZERFACTORY_H

#include "serializerinterface.h"
#include "patterns/factory/factory.h"

#include <QString>

class SerializerFactory : public Factory<ISerializer, QString>
{};

#endif // SERIALIZERFACTORY_H
