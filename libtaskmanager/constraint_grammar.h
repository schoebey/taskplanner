#ifndef CONSTRAINT_GRAMMAR_H
#define CONSTRAINT_GRAMMAR_H

#include "constraint.h"

namespace grammar
{
  // logical
  #define AND &
  #define BUT AND
  #define OR |

  // constraints
  #define ONE_OF(a, ...) tspConstraintTpl<decltype(a)>(make_list<decltype(a)>(a, ## __VA_ARGS__))
  #define MIN(a) tspConstraintTpl<decltype(a)>(std::make_shared<MinConstraint<decltype(a)>>(a))
  #define MAX(a) tspConstraintTpl<decltype(a)>(std::make_shared<MaxConstraint<decltype(a)>>(a))
  #define EVEN_NUMBER std::make_shared<ConstraintImpl>()


  template<typename T> tspConstraintTpl<T> operator&(const std::shared_ptr<ConstraintTpl<T>>& p,
                                                  const std::shared_ptr<ConstraintTpl<T>>& p2)
  {
    return std::make_shared<AndConstraint<T>>(p, p2);
  }
  template<typename T> tspConstraintTpl<T> operator|(tspConstraintTpl<T> p, tspConstraintTpl<T> p2)
  {
    return std::make_shared<OrConstraint<T>>(p, p2);
  }

  // todo: not operator: inverts the rhs constraint
}

#endif // CONSTRAINT_GRAMMAR_H
