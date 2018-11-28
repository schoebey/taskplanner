#ifndef PROPERTY_H
#define PROPERTY_H

#include <QString>

template<typename T> class Property
{
public:
  Property(const QString& sName, const T& data = T())
    : m_sName(sName),
      m_data(data)
  {}

  operator T() const
  {
    return m_data;
  }

  void operator= (const T& t)
  {
    m_data = t;
  }

private:
  QString m_sName;
  T m_data;
};

#endif // PROPERTY_H
