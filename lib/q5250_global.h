#ifndef Q5250_GLOBAL_H
#define Q5250_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(Q5250_LIBRARY)
#  define Q5250SHARED_EXPORT Q_DECL_EXPORT
#else
#  define Q5250SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // Q5250_GLOBAL_H
