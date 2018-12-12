#ifndef CONSTRAINTFACTORY_H
#define CONSTRAINTFACTORY_H

#include <QString>

#include <functional>
#include <map>

template<typename T> class ConstraintTpl;
template<typename T> using tspConstraintTpl = std::shared_ptr<ConstraintTpl<T>>;
template<typename T> using tFnCreator = std::function<tspConstraintTpl<T>(QString)>;

class ConstraintFactory
{
public:
  template<typename T> static tspConstraintTpl<T> create(const QString& sName,
                                                         const QString& sConfiguration)
  {
    auto creatorFunctions = creators<T>();
    auto it = creatorFunctions.find(sName);
    if (it != creatorFunctions.end())
    {
      return it->second(sConfiguration);
    }

    return nullptr;
  }

  template<typename T> static std::map<QString, tFnCreator<T>>& creators()
  {
    static std::map<QString, tFnCreator<T>> map;
    return map;
  }

  template<typename T> static void registerCreator(const QString& sName,
                                                   tFnCreator<T> fnCreator)
  {
    creators<T>()[sName] = fnCreator;
  }

private:
  ConstraintFactory() = delete;
};

#endif // CONSTRAINTFACTORY_H
