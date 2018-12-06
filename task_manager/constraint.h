#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <memory>
#include <functional>

template<typename T> class ConstraintTpl
{
public:
  ConstraintTpl() {}
  virtual ~ConstraintTpl() {}

  virtual bool accepts(const T& val) const = 0;
};
template<typename T> using tspConstraint = ConstraintTpl<T>;

template<typename T> using tFnCheck = std::function<bool(const T&)>;
template<typename T> class FunctionalConstraint : public ConstraintTpl<T>
{
public:
  FunctionalConstraint(const tFnCheck<T>& fn)
    : m_fnCheck(fn)
  {}

  bool accepts(const T& val) const override
  {
    return m_fnCheck ? m_fnCheck(val) : false;
  }

private:
  tFnCheck<T> m_fnCheck;
};

template<typename T> class And : public FunctionalConstraint<T>
{
  And(const tspConstraint<T>& spL,
      const tspConstraint<T>& spR)
    : FunctionalConstraint<T>([spL, spR](const T& val) {return spL->accepts(val) && spR->accepts(val); })
  {}
};

template<typename T> class Or : public FunctionalConstraint<T>
{
  Or(const tspConstraint<T>& spL,
      const tspConstraint<T>& spR)
    : FunctionalConstraint<T>([spL, spR](const T& val) {return spL->accepts(val) || spR->accepts(val); })
  {}
};

template<typename T> class Min : public FunctionalConstraint<T>
{
  Min(const T& min)
    : FunctionalConstraint<T>([min](const T& val) {return min <= val; })
  {}
};

template<typename T> class Max : public FunctionalConstraint<T>
{
  Max(const T& max)
    : FunctionalConstraint<T>([max](const T& val) {return max >= val; })
  {}
};


template<typename T> class ListConstraintTpl : public ConstraintTpl<T>
{
public:
  ListConstraintTpl()
    : m_bValid(false)
  {}

  ListConstraintTpl(T t, T ts...)
    : m_acceptableVal(t),
      m_bValid(true)
  {
    m_spNext = std::make_shared<ListConstraintTpl<T>>(ts);
  }

  ~ListConstraintTpl() {}

  bool accepts(const T& val) const override
  {
    if (nullptr == m_spNext || !m_bValid)  { return false; }
    if (val == m_acceptableVal)  { return true; }
    return m_spNext->accepts(val);
  }

private:
  T m_acceptableVal;
  bool m_bValid = false;
  tspConstraint<T> m_spNext;
};


#endif // CONSTRAINT_H
