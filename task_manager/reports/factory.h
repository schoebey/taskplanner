#ifndef FACTORY_H
#define FACTORY_H

#include <QString>

#include <map>
#include <vector>
#include <functional>


template<typename T> using tFnCreateObject = std::function<T*(void)>;

namespace detail
{
    template<typename Object, typename ObjectMetaInformation = bool>
    class FactoryPrivate
    {
        struct SInternalInfo
        {
          SInternalInfo() {}
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

      bool registerCreator(tFnCreateObject<Object> fnCreate,
                                                     const QString& sName,
                                                     const QString& sExtension)
      {
        SInternalInfo info(sName, sExtension, fnCreate);
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
              std::shared_ptr<Object>(it->second.fnCreator(), destroy);

          return spObject;
        }

        return nullptr;
      }

      void destroy(Object* pObject)
      {
        delete pObject;
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
  static detail::FactoryPrivate<ObjectMetaInformation, Object>* p()
  {
    static detail::FactoryPrivate<ObjectMetaInformation, Object> priv;
    return &priv;
  }
};


#endif // FACTORY_H
