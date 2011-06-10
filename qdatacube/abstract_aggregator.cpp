#include "abstract_aggregator.h"

#include "abstract_aggregator.moc"

qdatacube::abstract_aggregator_t::abstract_aggregator_t(QAbstractItemModel* model, QObject* parent):
    QObject(parent),
    m_underlying_model(model)
{
  Q_ASSERT(model);
}


