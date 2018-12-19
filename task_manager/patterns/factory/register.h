#ifndef REGISTER_H
#define REGISTER_H

#include "registrar.h"

#include <QString>

#include <functional>
#include <type_traits>

template<class T, class Interface, typename MetaInfo, class Enable = void>
class Register{};

template<class T, class Interface, typename MetaInfo>
class Register<T, Interface, MetaInfo, typename std::enable_if<std::is_base_of<Interface, T>::value>::type>
{
public:
  Register(const QString& sName, const MetaInfo& info = MetaInfo())
  {
    std::function<Interface*(void)> fn = []() { return new T(); };
    Registrar<Interface, QString>::registerCreator(fn, sName, info);
  }

  ~Register()
  {}
};

#endif // REGISTER_H
