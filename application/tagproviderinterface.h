#ifndef TAGPROVIDERINTERFACE_H
#define TAGPROVIDERINTERFACE_H

#include "id_types.h"

#include <QString>
#include <QColor>

#include <vector>

struct STagData
{
  tag_id id;
  QString sName;
  QColor color;
};

class ITagProvider
{
public:
  virtual std::vector<STagData> availableTags() const = 0;

protected:
  ITagProvider() {}
  ~ITagProvider() {}
};

#endif // TAGPROVIDERINTERFACE_H
