#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "serializableinterface.h"
#include "serializerinterface.h"
#include "conversion.h"
#include "constraintfactory.h"

#include <QDebug>

#include <memory>
#include <functional>


namespace detail
{
template<typename T, typename U>
std::shared_ptr<U> fnUnaryCreator(const QString& sName,
                                  const QString& sConfiguration)
{
  QRegExp rx("(.*):(.*)");
  if (-1 != rx.indexIn(sConfiguration))
  {
    if (sName == rx.cap(1))
    {
      QString sConfig = rx.cap(2);
      bool bDummy(false);
      return std::make_shared<U>(conversion::fromString<T>(sConfig, bDummy));
    }
  }
  return nullptr;
}

template<typename T, typename U>
std::shared_ptr<U> fnBinaryCreator(const QString& sName,
                                   const QString& sConfiguration)
{
  QRegExp rx(R"(([^:]+):\[([^\]]+)\];\[(.*)\])");
  if (-1 != rx.indexIn(sConfiguration))
  {
    QString sType = rx.cap(1);
    QString sLeftConfig = rx.cap(2);
    QString sRightConfig = rx.cap(3);
    if (sName == rx.cap(1))
    {
      QString sLeftName = sLeftConfig.left(sLeftConfig.indexOf(":"));
      QString sRightName = sRightConfig.left(sRightConfig.indexOf(":"));
      tspConstraintTpl<T> spL = ConstraintFactory::create<T>(sLeftName, rx.cap(2));
      tspConstraintTpl<T> spR = ConstraintFactory::create<T>(sRightName, rx.cap(3));

      return std::make_shared<U>(spL, spR);
    }
  }
  return nullptr;
}
}

//---------------------------------------------------------------------------------------
//
class IConstraint : public ISerializable
{
public:
  IConstraint() {}
  virtual ~IConstraint() {}

  virtual QString name() const = 0;

  virtual QString toString() const = 0;
};
using tspConstraint = std::shared_ptr<IConstraint>;

//---------------------------------------------------------------------------------------
//
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


//---------------------------------------------------------------------------------------
//
template<typename T> using tFnCheck = std::function<bool(const T&)>;
template<typename T> using tFnInterpolate = std::function<double(const T&)>;
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

//---------------------------------------------------------------------------------------
//
template<typename T> class AndConstraint : public FunctionalConstraint<T>
{
public:
  AndConstraint(const tspConstraintTpl<T>& spL,
      const tspConstraintTpl<T>& spR)
    : FunctionalConstraint<T>("And", [spL, spR](const T& val) {return spL->accepts(val) && spR->accepts(val); }),
      m_spL(spL),
      m_spR(spR)
  {

  }

  static void registerCreator()
  {
    tFnCreator<T> fn = std::bind(&detail::fnBinaryCreator<T, AndConstraint<T>>,
                                 QString("And"), std::placeholders::_1);
    ConstraintFactory::registerCreator("And", fn);
  }

  QString toString() const override
  {
    return QString("%1:[%2];[%3]").arg(this->name()).arg(m_spL->toString()).arg(m_spR->toString());
  }
private:
  tspConstraintTpl<T> m_spL;
  tspConstraintTpl<T> m_spR;
};

//---------------------------------------------------------------------------------------
//
template<typename T> class OrConstraint : public FunctionalConstraint<T>
{
public:
  OrConstraint(const tspConstraintTpl<T>& spL,
      const tspConstraintTpl<T>& spR)
    : FunctionalConstraint<T>("Or", [spL, spR](const T& val) {return spL->accepts(val) || spR->accepts(val); }),
      m_spL(spL),
      m_spR(spR)
  {}

  static void registerCreator()
  {
    tFnCreator<T> fn = std::bind(&detail::fnBinaryCreator<T, OrConstraint<T>>,
                                 QString("Or"), std::placeholders::_1);
    ConstraintFactory::registerCreator("Or", fn);
  }

  QString toString() const override
  {
    return QString("%1:[%2];[%3]").arg(this->name()).arg(m_spL->toString()).arg(m_spR->toString());
  }
private:
  tspConstraintTpl<T> m_spL;
  tspConstraintTpl<T> m_spR;
};

//---------------------------------------------------------------------------------------
//
template<typename T> class MinConstraint : public FunctionalConstraint<T>
{
public:
  MinConstraint(const T& min)
    : FunctionalConstraint<T>("Min", [min](const T& val) {return min <= val; }),
      m_min(min)
  {}

  static void registerCreator()
  {
    tFnCreator<T> fn = std::bind(&detail::fnUnaryCreator<T, MinConstraint<T>>,
                                 QString("Min"), std::placeholders::_1);
    ConstraintFactory::registerCreator("Min", fn);
  }

  QString toString() const override
  {
    return QString("%1:%2").arg(this->name()).arg(conversion::toString(m_min));
  }
private:
  T m_min;
};

//---------------------------------------------------------------------------------------
//
template<typename T> class MaxConstraint : public FunctionalConstraint<T>
{
public:
  MaxConstraint(const T& max)
    : FunctionalConstraint<T>("Max", [max](const T& val) {return max >= val; }),
      m_max(max)
  {}

  static void registerCreator()
  {
    tFnCreator<T> fn = std::bind(&detail::fnUnaryCreator<T, MaxConstraint<T>>,
                                 QString("Max"), std::placeholders::_1);
    ConstraintFactory::registerCreator("Max", fn);
  }

  QString toString() const override
  {
    return QString("%1:%2").arg(this->name()).arg(conversion::toString(m_max));
  }
private:
  T m_max;
};

//---------------------------------------------------------------------------------------
//
template<typename T> class EqualsConstraint : public FunctionalConstraint<T>
{
public:
  EqualsConstraint(const T& val)
    : FunctionalConstraint<T>("==", [val](const T& nominal) {return nominal == val; }),
      m_val(val)
  {}

  static void registerCreator()
  {
    tFnCreator<T> fn = std::bind(&detail::fnUnaryCreator<T, EqualsConstraint<T>>,
                                 QString("=="), std::placeholders::_1);
    ConstraintFactory::registerCreator("==", fn);
  }

  QString toString() const override
  {
    return QString("%1:%2").arg(this->name()).arg(conversion::toString(m_val));
  }
private:
  T m_val;
};

//---------------------------------------------------------------------------------------
//
template<typename T>
tspConstraintTpl<T> make_list(T t)
{
  return std::make_shared<EqualsConstraint<T>>(t);
}

//---------------------------------------------------------------------------------------
//
template<typename T, typename... Ts>
tspConstraintTpl<T> make_list(T t, Ts... ts)
{
  auto lhs = std::make_shared<EqualsConstraint<T>>(t);
  auto rhs = make_list(ts...);

  auto sp = std::make_shared<OrConstraint<T>>(lhs, rhs);
  return sp;
}

#endif // CONSTRAINT_H
