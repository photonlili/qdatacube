#include "abstract_filter.h"

namespace qdatacube {

abstract_filter_t::abstract_filter_t(QAbstractItemModel* underlying_model, QObject* parent)
  : QObject(parent),
    m_underlying_model(underlying_model)
{
  Q_ASSERT(underlying_model);

}

}

#include "abstract_filter.moc"
