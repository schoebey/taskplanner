#include "property.h"

namespace detail {
  template<typename T>
  T& factory(size_t scopeHashCode, size_t typeHashCode)
  {
    static std::map<size_t, std::map<size_t, T>> factories;

    auto it = factories.find(scopeHashCode);
    if (it == factories.end())
    {
      std::map<size_t, T> f;
      factories[scopeHashCode] = f;
      return factories[scopeHashCode][typeHashCode];
    }
    else
    {
      auto it2 = it->second.find(typeHashCode);
      if (it2 == it->second.end())
      {
        T f;
        it->second[typeHashCode] = f;
        it2 = it->second.find(typeHashCode);
      }

      return it2->second;
    }
  }
}

factory::tProperties& factory::propertiesFactory(size_t scopeHashCode, size_t typeHashCode)
{
  return detail::factory<factory::tProperties>(scopeHashCode, typeHashCode);
}

factory::tDescriptors &factory::descriptorsFactory(size_t scopeHashCode, size_t typeHashCode)
{
  return detail::factory<factory::tDescriptors>(scopeHashCode, typeHashCode);
}
