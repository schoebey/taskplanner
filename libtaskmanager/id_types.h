#ifndef ID_TYPES_H
#define ID_TYPES_H

#include <QString>

template<class Tag, class T>
class ID
{
public:
    ID(T val = T()) : m_val(val) { }

    operator T() const { return m_val; }

//    friend bool operator==(ID a, ID b) { return a.m_val == b.m_val; }
    friend bool operator!=(ID a, ID b) { return a.m_val != b.m_val; }
    friend bool operator>(ID a, ID b) { return a.m_val > b.m_val; }
    friend bool operator>=(ID a, ID b) { return a.m_val >= b.m_val; }
    friend bool operator<(ID a, ID b) { return a.m_val < b.m_val; }
    friend bool operator<=(ID a, ID b) { return a.m_val <= b.m_val; }
    ID& operator++() { m_val++; return *this; }
    ID operator++(int) { ID ret = *this; this->operator++(); return ret; }

private:
    T m_val;
};


struct task_id_tag;
typedef ID<task_id_tag, int> task_id;
struct group_id_tag;
typedef ID<group_id_tag, int> group_id;
struct tag_id_tag;
typedef ID<tag_id_tag, int> tag_id;

#endif // ID_TYPES_H
