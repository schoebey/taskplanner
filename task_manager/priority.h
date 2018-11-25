#ifndef PRIORITY_H
#define PRIORITY_H

#include <vector>

struct SPriority
{
  SPriority()
    : categories({0})
  {}

  std::vector<int> categories;
};

#endif // PRIORITY_H
