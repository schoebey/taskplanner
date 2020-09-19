#include "tagprovider.h"
#include "manager.h"

#include <taginterface.h>

TagProvider::TagProvider(Manager* pManager)
  : m_pManager(pManager)
{
}

TagProvider::~TagProvider()
{
}

std::vector<STagData> TagProvider::availableTags() const
{
  std::vector<STagData> vData;

  for (const auto& tagId : m_pManager->tagIds())
  {
    ITag* pTag = m_pManager->tag(tagId);
    if (nullptr != pTag) {
      vData.push_back({pTag->id(), pTag->name(), pTag->color()});
    }
  }

  return vData;
}

tag_id TagProvider::addTag()
{
  auto pTag = m_pManager->addTag();
  if (nullptr != pTag)
  {
    return pTag->id();
  }

  return tag_id{};
}

bool TagProvider::deleteTag(tag_id tagId)
{
  return m_pManager->removeTag(tagId);
}

bool TagProvider::modifyTag(tag_id tagId, const QString& sName, const QColor& color)
{
  return m_pManager->modifyTag(tagId, sName, color);
}
