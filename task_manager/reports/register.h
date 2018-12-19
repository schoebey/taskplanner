#ifndef REGISTER_H
#define REGISTER_H

#include <QString>

#include <functional>
#include <type_traits>

template<class T, class Interface, class Registrar, class Enable = void>
class Register{};

template<class T, class Interface, class Registrar>
class Register<T, Interface, Registrar, typename std::enable_if<std::is_base_of<Interface, T>::value>::type>
{
public:
  Register(const QString& sName, const QString& sExtension)
  {
    std::function<Interface*(void)> fn = []() { return new T(); };
    Registrar::registerCreator(fn, sName, sExtension);
  }

  ~Register()
  {}
};

#endif // REGISTER_H
