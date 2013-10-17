#include "filterbyaggregate.h"
#include "abstractaggregator.h"

namespace qdatacube {

class FilterByAggregatePrivate {
    public:
        FilterByAggregatePrivate(std::tr1::shared_ptr<AbstractAggregator> aggregator, int category_index) : m_aggregator(aggregator), m_categoryIndex(category_index) {}
        std::tr1::shared_ptr<AbstractAggregator> m_aggregator;
        int m_categoryIndex;
};

FilterByAggregate::FilterByAggregate(std::tr1::shared_ptr< AbstractAggregator > aggregator, int category_index)
 : AbstractFilter(aggregator->underlyingModel()), d(new FilterByAggregatePrivate(aggregator, category_index))
{
  Q_ASSERT(aggregator);
  Q_ASSERT(category_index>=0);
  Q_ASSERT(category_index<aggregator->categoryCount());
  connect(aggregator.get(), SIGNAL(categoryAdded(int)), SLOT(slot_aggregator_category_inserted(int)));
  connect(aggregator.get(), SIGNAL(categoryRemoved(int)), SLOT(slot_aggregator_category_removed(int)));
  setShortName(d->m_aggregator->categoryHeaderData(d->m_categoryIndex).toString());
  setName(d->m_aggregator->name() + "=" + d->m_aggregator->categoryHeaderData(d->m_categoryIndex).toString());
}

bool FilterByAggregate::operator()(int row) const {
  return ((*(d->m_aggregator))(row) == d->m_categoryIndex);
}

void FilterByAggregate::slot_aggregator_category_inserted(int index) {
  if (index <= d->m_categoryIndex) {
    ++d->m_categoryIndex;
  }
}

void FilterByAggregate::slot_aggregator_category_removed(int index) {
  if (index == d->m_categoryIndex) {
    d->m_categoryIndex = -1;
  } else if (index<d->m_categoryIndex) {
    --d->m_categoryIndex;
  }
}

std::tr1::shared_ptr< AbstractAggregator > FilterByAggregate::aggregator() const {
    return d->m_aggregator;
}

int FilterByAggregate::categoryIndex() const {
    return d->m_categoryIndex;
}

FilterByAggregate::~FilterByAggregate() {

}



}
#include "filterbyaggregate.moc"
