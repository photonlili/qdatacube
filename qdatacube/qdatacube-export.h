#include <QtCore/QtGlobal>

#ifndef QDATACUBE_EXPORT
#  if defined(BUILDING_QDATACUBE)
#    define QDATACUBE_EXPORT Q_DECL_EXPORT
#  else
#    define QDATACUBE_EXPORT Q_DECL_IMPORT
#  endif
#endif