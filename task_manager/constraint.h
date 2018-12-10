#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "serializableinterface.h"
#include "serializerinterface.h"
#include "conversion.h"

#include <QDebug>

#include <memory>
#include <functional>

class IConstraint : public ISerializable
{
public:
  IConstraint() {}
  virtual ~IConstraint() {}

  virtual QString name() const = 0;

  virtual QString toString() const = 0;
};
using tspConstraint = std::shared_ptr<IConstraint>;

template<typename T> class ConstraintTpl : public IConstraint
{
public:
  ConstraintTpl(const QString& sName) : m_sName(sName) {}
  virtual ~ConstraintTpl() {}

  virtual bool accepts(const T& val) const = 0;

  QString name() const override
  {
    return m_sName;
  }

  int version() const override
  {
    return 0;
  }

  ESerializingError serialize(ISerializer* pSerializer) const override
  {
    return pSerializer->serialize(*this);
  }

  EDeserializingError deserialize(ISerializer* pSerializer) override
  {
    return pSerializer->deserialize(*this);
  }

private:
  QString m_sName;
};
template<typename T> using tspConstraintTpl = std::shared_ptr<ConstraintTpl<T>>;

template<typename T> using tFnCheck = std::function<bool(const T&)>;
template<typename T> class FunctionalConstraint : public ConstraintTpl<T>
{
public:
  FunctionalConstraint(const QString& sName, const tFnCheck<T>& fn)
    : ConstraintTpl<T>(sName),
      m_fnCheck(fn)
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
  AndConstraint(const tspConstraintTpl<T>& spL,
      const tspConstraintTpl<T>& spR)
    : FunctionalConstraint<T>("And", [spL, spR](const T& val) {return spL->accepts(val) && spR->accepts(val); }),
      m_spL(spL),
      m_spR(spR)
  {}

  QString toString() const override
  {
    return m_spL->toString() + ";" + m_spR->toString();
  }
private:
  tspConstraintTpl<T> m_spL;
  tspConstraintTpl<T> m_spR;
};

template<typename T> class OrConstraint : public FunctionalConstraint<T>
{
public:
  OrConstraint(const tspConstraintTpl<T>& spL,
      const tspConstraintTpl<T>& spR)
    : FunctionalConstraint<T>("Or", [spL, spR](const T& val) {return spL->accepts(val) || spR->accepts(val); }),
      m_spL(spL),
      m_spR(spR)
  {}

  QString toString() const override
  {
    return m_spL->toString() + ";" + m_spR->toString();
  }
private:
  tspConstraintTpl<T> m_spL;
  tspConstraintTpl<T> m_spR;
};

template<typename T> class MinConstraint : public FunctionalConstraint<T>
{
public:
  MinConstraint(const T& min)
    : FunctionalConstraint<T>("Min", [min](const T& val) {return min <= val; }),
      m_min(min)
  {}

  QString toString() const override
  {
    return QString::number(m_min);
  }
private:
  T m_min;
};

template<typename T> class MaxConstraint : public FunctionalConstraint<T>
{
public:
  MaxConstraint(const T& max)
    : FunctionalConstraint<T>("Max", [max](const T& val) {return max >= val; }),
      m_max(max)
  {}

  QString toString() const override
  {
    return QString::number(m_max);
  }
private:
  T m_max;
};


template<typename T>
class ListConstraintTpl : public ConstraintTpl<T>
{
public:
  ListConstraintTpl()
    : ConstraintTpl<T>("List")
  {
  }

  ListConstraintTpl(T t)
    : ConstraintTpl<T>("List"),
      m_acceptableVal(t)
  {
  }

  ~ListConstraintTpl() {}

  void setNext(const tspConstraintTpl<T>& sp)
  {
    m_spNext = sp;
  }

  bool accepts(const T& val) const override
  {
    if (val == m_acceptableVal)  { return true; }
    if (nullptr == m_spNext)  { return false; }
    return m_spNext->accepts(val);
  }

  QString toString() const override
  {
    QString sRv = conversion::toString(m_acceptableVal);
    if (m_spNext)
    {
      sRv += ";" + m_spNext->toString();
    }
    return sRv;
  }

private:
  T m_acceptableVal;
  tspConstraintTpl<T> m_spNext;
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
