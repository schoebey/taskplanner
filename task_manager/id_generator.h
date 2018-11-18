#ifndef ID_GENERATOR_H
#define ID_GENERATOR_H

template<class T> class id_generator
{
  T generate(T id)
  {
    if (-1 == id)
    {
      return c_nextId++;
    }
    else if (id > c_nextId)
    {
      c_nextId = id;
    }

    return id;
  }
public:
  id_generator(T id)
    : m_id(generate(id))
  {}

  T id() const
  {
    return m_id;
  }

  void setId(T id)
  {
    m_id = id;
    if (c_nextId <= id)
    {
      c_nextId = id + 1;
    }
  }

protected:
  T m_id;
  static T c_nextId;
};

template<class T> T id_generator<T>::c_nextId = 0;

#endif // ID_GENERATOR_H
