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

  virtual tag_id addTag() = 0;

  virtual bool deleteTag(tag_id tagId) = 0;

  virtual bool modifyTag(tag_id tagId, const QString& sName, const QColor& color) = 0;

protected:
  ITagProvider() {}
  ~ITagProvider() {}
};

#endif // TAGPROVIDERINTERFACE_H
