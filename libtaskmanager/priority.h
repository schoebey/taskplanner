#ifndef PRIORITY_H
#define PRIORITY_H

#include <vector>

struct SPriority
{
  SPriority()
    : categories({0})
  {}

  void setPriority(size_t iCategory, int iPriority)
  {
    if (iCategory >= categories.size())
    {
      categories.resize(iCategory + 1, -1);
    }

    categories[iCategory] = iPriority;
  }

  int priority(size_t iCategory) const
  {
    if (iCategory < categories.size())
    {
      return categories[iCategory];
    }

    return -1;
  }

  std::vector<int> categories;
};


#endif // PRIORITY_H
