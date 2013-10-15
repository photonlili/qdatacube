#include "abstract_aggregator.h"

class qdatacube::AbstractAggregatorPrivate {
    public:
        AbstractAggregatorPrivate(QAbstractItemModel* underlying_model) : m_underlying_model(underlying_model), m_name("unnamed") {
        }
        QAbstractItemModel* m_underlying_model;
        QString m_name;
};

qdatacube::abstract_aggregator_t::abstract_aggregator_t(QAbstractItemModel* model):
    d(new AbstractAggregatorPrivate(model))
{
  Q_ASSERT(model);
}

QString qdatacube::abstract_aggregator_t::name() const {
    return d->m_name;
}

QAbstractItemModel* qdatacube::abstract_aggregator_t::underlying_model() const {
    return d->m_underlying_model;
}

qdatacube::abstract_aggregator_t::~abstract_aggregator_t() {

}

void qdatacube::abstract_aggregator_t::setName(const QString& newName) {
    d->m_name = newName;
}


#include "abstract_aggregator.moc"
