#ifndef REGISTRAR_H
#define REGISTRAR_H

#include "factory.h"

#include <QString>

#include <functional>
#include <type_traits>

template<typename T, typename MetaInfo>
class Registrar : public Factory<T, MetaInfo>
{
public:
    Registrar() {}

  static bool registerCreator(tFnCreateObject<T> fnCreate, const QString& sName,
                              const MetaInfo& info)
  {
    return Factory<T, MetaInfo>::p()->registerCreator(fnCreate, sName, info);
  }
};

#endif // REGISTRAR_H
