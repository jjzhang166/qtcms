#ifndef QARP_GLOBAL_H
#define QARP_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef QARP_LIB
# define QARP_EXPORT Q_DECL_EXPORT
#else
# define QARP_EXPORT Q_DECL_IMPORT
#endif

#endif // QARP_GLOBAL_H
