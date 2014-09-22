#include "filterbyaggregate.h"
#include "abstractaggregator.h"
#include <QSharedPointer>

namespace qdatacube {

class FilterByAggregatePrivate {
    public:
        FilterByAggregatePrivate(AbstractAggregator::Ptr aggregator, int category_index) : m_aggregator(aggregator), m_categoryIndex(category_index) {}
        AbstractAggregator::Ptr m_aggregator;
        int m_categoryIndex;
};

FilterByAggregate::FilterByAggregate(AbstractAggregator::Ptr aggregator, int category_index)
 : AbstractFilter(aggregator->underlyingModel()), d(new FilterByAggregatePrivate(aggregator, category_index))
{
  Q_ASSERT(aggregator);
  Q_ASSERT(category_index>=0);
  Q_ASSERT(category_index<aggregator->categoryCount());
  connect(aggregator.data(), SIGNAL(categoryAdded(int)), SLOT(slot_aggregator_category_inserted(int)));
  connect(aggregator.data(), SIGNAL(categoryRemoved(int)), SLOT(slot_aggregator_category_removed(int)));
  setShortName(d->m_aggregator->categoryHeaderData(d->m_categoryIndex).toString());
  setName(d->m_aggregator->name() + "=" + d->m_aggregator->categoryHeaderData(d->m_categoryIndex).toString());
}

/**
 * utility function to find the index corresponding to a category
 */
int categoryToIndex(const AbstractAggregator::Ptr aggregator, const QVariant& category) {
    int categoryIndex = -1;
    for(int i = 0 ; i < aggregator->categoryCount(); i++ ) {
        if(aggregator->categoryHeaderData(i) == category ) {
            categoryIndex = i;
            break;
        }
    }
    return categoryIndex;
}

FilterByAggregate::FilterByAggregate(AbstractAggregator::Ptr aggregator, const QVariant& category):
  FilterByAggregate(aggregator, categoryToIndex(aggregator, category)) {}


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

AbstractAggregator::Ptr FilterByAggregate::aggregator() const {
    return d->m_aggregator;
}

int FilterByAggregate::categoryIndex() const {
    return d->m_categoryIndex;
}

FilterByAggregate::~FilterByAggregate() {

}



}
#include "filterbyaggregate.moc"
