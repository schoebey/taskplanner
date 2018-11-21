#ifndef PRIORITY_H
#define PRIORITY_H

#include <array>

struct SPriority
{
  SPriority()
    : categories({{0}})
  {}

  std::array<int, 10> categories;
};

#endif // PRIORITY_H
