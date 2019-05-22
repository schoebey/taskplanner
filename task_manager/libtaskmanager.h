#ifndef LIBTASKMANAGER_H
#define LIBTASKMANAGER_H

#include <QtGlobal>

#ifdef BUILD_LIB
  #define LIBTASKMANAGER Q_DECL_EXPORT
#else
  #define LIBTASKMANAGER Q_DECL_IMPORT
#endif


#endif // LIBTASKMANAGER_H
