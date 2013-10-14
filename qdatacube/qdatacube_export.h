#include <QtCore/QtGlobal>

#ifndef QDATACUBE_EXPORT
#  if defined(BUILDING_QDATACUBE)
#    define QDATACUBE_EXPORT Q_DECL_EXPORT
#    define QDATACUBE_DEPRECATED __attribute__((deprecated))
#  else
#    define QDATACUBE_EXPORT Q_DECL_IMPORT
#    define QDATACUBE_DEPRECATED __attribute__((deprecated))
#  endif
#endif
