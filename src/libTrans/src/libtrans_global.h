#ifndef LIBTRANS_GLOBAL_H
#define LIBTRANS_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef LIBTRANS_LIB
# define LIBTRANS_EXPORT Q_DECL_EXPORT
#else
# define LIBTRANS_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBTRANS_GLOBAL_H
