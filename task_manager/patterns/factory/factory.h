#ifndef FACTORY_H
#define FACTORY_H

#include "libtaskmanager.h"

#include <QString>

#include <map>
#include <vector>
#include <functional>


template<typename T> using tFnCreateObject = std::function<T*(void)>;

namespace detail
{
   class LIBTASKMANAGER FactoryPrivateBase
   {
   public:
     FactoryPrivateBase();
     virtual ~FactoryPrivateBase();
   };
   typedef std::shared_ptr<FactoryPrivateBase> tspPrivBase;

   tspPrivBase LIBTASKMANAGER p_base(size_t objectHashCode, size_t metaInfoHashCode);

   void LIBTASKMANAGER reg_base(const detail::tspPrivBase& spPriv, size_t objectHashCode, size_t metaInfoHashCode);

    template<typename Object, typename ObjectMetaInformation>
    class FactoryPrivate : public FactoryPrivateBase
    {
        struct SInternalInfo
        {
          SInternalInfo() {}
          ~SInternalInfo() {}
          SInternalInfo(const QString& sName,
                        const ObjectMetaInformation& info,
                        tFnCreateObject<Object> fnCreate)
            : sName(sName), info(info), fnCreator(fnCreate)
          {}
          QString sName;
          ObjectMetaInformation info;
          tFnCreateObject<Object> fnCreator;
        };

    public:
        using tspObject = std::shared_ptr<Object>;

        FactoryPrivate() {}

        ~FactoryPrivate() override {}

      bool registerCreator(tFnCreateObject<Object> fnCreate,
                                                     const QString& sName,
                                                     const ObjectMetaInformation& metaInfo)
      {
        SInternalInfo info(sName, metaInfo, fnCreate);
        if (creators.find(sName) != creators.end())  { return false; }
        creators[sName] = info;

        return true;
      }

      std::map<QString, ObjectMetaInformation> classes()
      {
        std::map<QString, ObjectMetaInformation> infoMap;
        for (const auto& info : creators)
        {
          infoMap[info.first] = info.second.info;
        }

        return infoMap;
      }

      tspObject create(const QString& sName)
      {
        // look up creator using sName
        auto it = creators.find(sName);
        if (it != creators.end())
        {
          tspObject spObject =
              std::shared_ptr<Object>(it->second.fnCreator());

          return spObject;
        }

        return nullptr;
      }


    private:
      std::map<QString, SInternalInfo> creators;
    };
}

template<typename Object, typename ObjectMetaInformation>
class Factory
{
public:
  using tspObject = std::shared_ptr<Object>;

  Factory();

  static std::map<QString, ObjectMetaInformation> classes()
  {
    return p()->classes();
  }

  static tspObject create(const QString& sName)
    {
    return p()->create(sName);
  }

protected:
  static detail::FactoryPrivate<Object, ObjectMetaInformation>* p()
  {
    detail::tspPrivBase spPriv = detail::p_base(typeid(Object).hash_code(), typeid(ObjectMetaInformation).hash_code());
    if (nullptr == spPriv)
    {
      spPriv = std::make_shared<detail::FactoryPrivate<Object, ObjectMetaInformation>>();
      detail::reg_base(spPriv, typeid(Object).hash_code(), typeid(ObjectMetaInformation).hash_code());
    }

    detail::FactoryPrivateBase* pBase = spPriv.get();

    return dynamic_cast<detail::FactoryPrivate<Object, ObjectMetaInformation>*>(pBase);
  }
};


#endif // FACTORY_H
