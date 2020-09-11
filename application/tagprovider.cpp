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
