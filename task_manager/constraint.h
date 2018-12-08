#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <QDebug>

#include <memory>
#include <functional>

template<typename T> class ConstraintTpl
{
public:
  ConstraintTpl() {}
  virtual ~ConstraintTpl() {}

  virtual bool accepts(const T& val) const = 0;
};
template<typename T> using tspConstraint = std::shared_ptr<ConstraintTpl<T>>;

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

template<typename T> class AndConstraint : public FunctionalConstraint<T>
{
public:
  AndConstraint(const tspConstraint<T>& spL,
      const tspConstraint<T>& spR)
    : FunctionalConstraint<T>([spL, spR](const T& val) {return spL->accepts(val) && spR->accepts(val); })
  {}
};

template<typename T> class OrConstraint : public FunctionalConstraint<T>
{
public:
  OrConstraint(const tspConstraint<T>& spL,
      const tspConstraint<T>& spR)
    : FunctionalConstraint<T>([spL, spR](const T& val) {return spL->accepts(val) || spR->accepts(val); })
  {}
};

template<typename T> class MinConstraint : public FunctionalConstraint<T>
{
public:
  MinConstraint(const T& min)
    : FunctionalConstraint<T>([min](const T& val) {return min <= val; })
  {}
};

template<typename T> class MaxConstraint : public FunctionalConstraint<T>
{
public:
  MaxConstraint(const T& max)
    : FunctionalConstraint<T>([max](const T& val) {return max >= val; })
  {}
};


template<typename T>
class ListConstraintTpl : public ConstraintTpl<T>
{
public:
  ListConstraintTpl()
  {
    qDebug() << "end";
  }

  ListConstraintTpl(T t)
    : m_acceptableVal(t)
  {
    qDebug() << t;
  }

  ~ListConstraintTpl() {}

  void setNext(const tspConstraint<T>& sp)
  {
    m_spNext = sp;
  }

  bool accepts(const T& val) const override
  {
    if (val == m_acceptableVal)  { return true; }
    if (nullptr == m_spNext)  { return false; }
    return m_spNext->accepts(val);
  }

private:
  T m_acceptableVal;
  tspConstraint<T> m_spNext;
};

template<typename T>
std::shared_ptr<ListConstraintTpl<T>> make_list(T t)
{
  return std::make_shared<ListConstraintTpl<T>>(t);;
}

template<typename T, typename... Ts>
std::shared_ptr<ListConstraintTpl<T>> make_list(T t, Ts... ts)
{
  auto sp = std::make_shared<ListConstraintTpl<T>>(t);
  sp->setNext(make_list(ts...));
  return sp;
}

#endif // CONSTRAINT_H
