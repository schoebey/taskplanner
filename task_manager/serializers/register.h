#ifndef REGISTER_H
#define REGISTER_H

#include "serializerregistrar.h"
#include "serializerinterface.h"

template<class T> class Register
{
public:
  Register(const QString& sName, const QString& sExtension)
  {
    std::function<ISerializer*(void)> fn = []() { return new T(); };
    SerializerRegistrar::registerCreator(fn, sName, sExtension);
  }

  ~Register()
  {}
};

#endif // REGISTER_H
