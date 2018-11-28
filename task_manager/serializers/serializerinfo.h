#ifndef SERIALIZERINFO_H
#define SERIALIZERINFO_H

#include <QString>

#include <functional>

class ISerializer;
typedef std::function<ISerializer*(void)> tFnCreate;

struct SSerializerInfo
{
  SSerializerInfo() {}

  SSerializerInfo(const QString& sName, const QString& sExtension)
    : sName(sName), sFileExtension(sExtension)
  {}



  QString sName;
  QString sFileExtension;
};

struct SInternalInfo
{
  SInternalInfo() {}
  SInternalInfo(const QString& sName, const QString& sExtension, tFnCreate fnCreate)
    : info(sName, sExtension), fnCreator(fnCreate)
  {}
  SSerializerInfo info;
  tFnCreate fnCreator;
};

#endif // SERIALIZERINFO_H
