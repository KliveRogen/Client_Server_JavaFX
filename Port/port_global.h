#ifndef PORT_GLOBAL_H
#define PORT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PORT_LIBRARY)
#  define PORTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PORTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PORT_GLOBAL_H
