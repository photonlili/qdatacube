#include "cell.h"

namespace qdatacube {

QDebug operator<<(QDebug dbg, const cell_t& cell)
{
  dbg.nospace() << "(" << cell.row() << "," << cell.column() << ")";
  return dbg.space();
}


} // end of namespace

