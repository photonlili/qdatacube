#include "abstract_aggregator.h"

class qdatacube::AbstractAggregatorPrivate {
    public:
        AbstractAggregatorPrivate(QAbstractItemModel* underlying_model) : m_underlying_model(underlying_model) {
        }
        QAbstractItemModel* m_underlying_model;
};

qdatacube::abstract_aggregator_t::abstract_aggregator_t(QAbstractItemModel* model):
    d(new AbstractAggregatorPrivate(model))
{
  Q_ASSERT(model);
}

QString qdatacube::abstract_aggregator_t::name() const {
    return QLatin1String("unnamed");
}

QAbstractItemModel* qdatacube::abstract_aggregator_t::underlying_model() const {
    return d->m_underlying_model;
}

qdatacube::abstract_aggregator_t::~abstract_aggregator_t() {

}

#include "abstract_aggregator.moc"
