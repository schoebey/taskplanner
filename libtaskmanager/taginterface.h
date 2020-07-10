#ifndef TAGINTERFACE_H
#define TAGINTERFACE_H

#include "id_types.h"
#include "id_generator.h"
#include "propertyproviderinterface.h"

#include <QString>
#include <QColor>

class ITag : public id_generator<tag_id>
{
public:
  virtual QString name() const = 0;
  virtual void setName(const QString& sName) = 0;

  virtual QColor color() const = 0;
  virtual void setColor(const QColor&) = 0;

protected:
  ITag(tag_id id = -1) : id_generator<tag_id>(id) {}
  ~ITag() {}
};

#endif // TAGINTERFACE_H
