#include "filter_by_aggregate.h"
#include "abstract_aggregator.h"

namespace qdatacube {

class FilterByAggregatePrivate {
    public:
        FilterByAggregatePrivate(std::tr1::shared_ptr<abstract_aggregator_t> aggregator, int category_index) : m_aggregator(aggregator), m_categoryIndex(category_index) {}
        std::tr1::shared_ptr<abstract_aggregator_t> m_aggregator;
        int m_categoryIndex;
};

filter_by_aggregate_t::filter_by_aggregate_t(std::tr1::shared_ptr< abstract_aggregator_t > aggregator, int category_index, QObject* parent)
 : abstract_filter_t(aggregator->underlying_model(), parent), d(new FilterByAggregatePrivate(aggregator, category_index))
{
  Q_ASSERT(aggregator);
  Q_ASSERT(category_index>=0);
  Q_ASSERT(category_index<aggregator->categoryCount());
  connect(aggregator.get(), SIGNAL(category_added(int)), SLOT(slot_aggregator_category_inserted(int)));
  connect(aggregator.get(), SIGNAL(category_removed(int)), SLOT(slot_aggregator_category_removed(int)));
  setShortName(d->m_aggregator->categoryHeaderData(d->m_categoryIndex).toString());
  setName(d->m_aggregator->name() + "=" + d->m_aggregator->categoryHeaderData(d->m_categoryIndex).toString());
}

bool filter_by_aggregate_t::operator()(int row) const {
  return ((*(d->m_aggregator))(row) == d->m_categoryIndex);
}

void filter_by_aggregate_t::slot_aggregator_category_inserted(int index) {
  if (index <= d->m_categoryIndex) {
    ++d->m_categoryIndex;
  }
}

void filter_by_aggregate_t::slot_aggregator_category_removed(int index) {
  if (index == d->m_categoryIndex) {
    d->m_categoryIndex = -1;
  } else if (index<d->m_categoryIndex) {
    --d->m_categoryIndex;
  }
}

std::tr1::shared_ptr< abstract_aggregator_t > filter_by_aggregate_t::aggregator() const {
    return d->m_aggregator;
}

int filter_by_aggregate_t::category_index() const {
    return d->m_categoryIndex;
}

filter_by_aggregate_t::~filter_by_aggregate_t() {

}



}
#include "filter_by_aggregate.moc"
