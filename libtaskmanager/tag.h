#ifndef TAG_H
#define TAG_H

#include "libtaskmanager.h"

#include "taginterface.h"
#include "id_types.h"
#include "serializableinterface.h"

#include <QString>

#include <set>
#include <memory>

class Manager;

class LIBTASKMANAGER Tag: public ITag, public ISerializable
{
public:
  using id_generator<tag_id>::setId;

  Tag(tag_id id = -1);

  int version() const override;

  QString name() const override;
  void setName(const QString& sName) override;

  QColor color() const override;
  void setColor(const QColor&) override;

  ESerializingError serialize(ISerializer* pSerializer) const override;

  EDeserializingError deserialize(ISerializer* pSerializer) override;

private:
  QString m_sName;
  QColor m_color;
};

#endif // TAG_H
