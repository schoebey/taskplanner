#ifndef ID_GENERATOR_H
#define ID_GENERATOR_H

template<class T> class id_generator
{
public:
  id_generator()
    : m_id(c_nextId++)
  {}

protected:
  T m_id;
  static T c_nextId;
};

template<class T> T id_generator<T>::c_nextId = 0;

#endif // ID_GENERATOR_H
