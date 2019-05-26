#include "factory.h"

namespace  {
  std::map<size_t, std::map<size_t, detail::tspPrivBase>> factories;
}

namespace detail
{
  tspPrivBase p_base(size_t objectHashCode, size_t metaInfoHashCode)
  {
    auto it = factories.find(objectHashCode);
    if (it == factories.end())
    {
      std::map<size_t, detail::tspPrivBase> f;
      factories[objectHashCode] = f;
      return factories[objectHashCode][metaInfoHashCode];
    }
    else
    {
      auto it2 = it->second.find(metaInfoHashCode);
      if (it2 == it->second.end())
      {
        return nullptr;
      }

      return it2->second;
    }
  }

  void reg_base(const detail::tspPrivBase& spPriv, size_t objectHashCode, size_t metaInfoHashCode)
  {
    factories[objectHashCode][metaInfoHashCode] = spPriv;
  }

  FactoryPrivateBase::FactoryPrivateBase()
  {

  }

  FactoryPrivateBase::~FactoryPrivateBase()
  {

  }

}
