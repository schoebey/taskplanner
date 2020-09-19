#ifndef TAGPROVIDER_H
#define TAGPROVIDER_H

#include "tagproviderinterface.h"

class Manager;
class TagProvider : public ITagProvider
{
public:
  TagProvider(Manager* pManager);
  virtual ~TagProvider();

  std::vector<STagData> availableTags() const override;

  tag_id addTag() override;

  bool deleteTag(tag_id tagId) override;

  bool modifyTag(tag_id tagId, const QString& sName, const QColor& color) override;

private:
  Manager* m_pManager;
};

#endif // TAGPROVIDER_H
