#ifndef DYNMATH_GLOBAL_H
#define DYNMATH_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DYNMATH_LIBRARY)
#  define DYNMATHSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DYNMATHSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // DYNMATH_GLOBAL_H
