#ifndef COMPAT_H
#define COMPAT_H

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace Qt
{
  static auto endl = ::endl;
}
#endif


#endif // COMPAT_H
