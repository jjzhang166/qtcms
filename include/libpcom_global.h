#ifndef LIBPCOM_GLOBAL_H
#define LIBPCOM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBPCOM_LIBRARY)
#  define LIBPCOMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBPCOMSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBPCOM_GLOBAL_H
