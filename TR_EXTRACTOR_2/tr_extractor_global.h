#ifndef TR_EXTRACTOR_GLOBAL_H
#define TR_EXTRACTOR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TR_EXTRACTOR_LIBRARY)
#  define TR_EXTRACTORSHARED_EXPORT Q_DECL_EXPORT
#else
#  define TR_EXTRACTORSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // TR_EXTRACTOR_GLOBAL_H
