#include "tag.h"

#include "serializerinterface.h"

Tag::Tag(tag_id id)
  : ITag(id)
{
}

int Tag::version() const
{
  return 0;
}

QString Tag::name() const
{
  return m_sName;
}

void Tag::setName(const QString& sName)
{
  m_sName = sName;
}

QColor Tag::color() const
{
  return m_color;
}

void Tag::setColor(const QColor& color)
{
  m_color = color;
}

ESerializingError Tag::serialize(ISerializer* pSerializer) const
{
  return pSerializer->serialize(*this);
}

EDeserializingError Tag::deserialize(ISerializer* pSerializer)
{
  return pSerializer->deserialize(*this);
}
