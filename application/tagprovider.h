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

private:
  Manager* m_pManager;
};

#endif // TAGPROVIDER_H
