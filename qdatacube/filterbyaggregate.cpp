#include "filterbyaggregate.h"

#include "abstractaggregator.h"
#include <QSharedPointer>

namespace qdatacube {

class FilterByAggregatePrivate {
public:
    FilterByAggregatePrivate(FilterByAggregate* q, AbstractAggregator::Ptr aggregator, int category_index)
      : m_aggregator(aggregator), m_categoryIndex(category_index)
    {
        Q_ASSERT(aggregator);
        Q_ASSERT(category_index<aggregator->categoryCount());
        QObject::connect(aggregator.data(), &AbstractAggregator::categoryAdded, q, &FilterByAggregate::slot_aggregator_category_inserted);
        QObject::connect(aggregator.data(), &AbstractAggregator::categoryRemoved, q, &FilterByAggregate::slot_aggregator_category_removed);
        q->setShortName(m_aggregator->categoryHeaderData(m_categoryIndex).toString());
        q->setName(m_aggregator->name() + "=" + m_aggregator->categoryHeaderData(m_categoryIndex).toString());
    }

    AbstractAggregator::Ptr m_aggregator;
    int m_categoryIndex;
};

FilterByAggregate::FilterByAggregate(AbstractAggregator::Ptr aggregator, int category_index)
 : AbstractFilter(aggregator->underlyingModel()), d(new FilterByAggregatePrivate(this, aggregator, category_index)) {
     Q_ASSERT(category_index >= 0);
}

int categoryToIndex(const AbstractAggregator::Ptr aggregator, const QString& category);

FilterByAggregate::FilterByAggregate(AbstractAggregator::Ptr aggregator, const QString& category)
 : AbstractFilter(aggregator->underlyingModel()), d(new FilterByAggregatePrivate(this, aggregator, categoryToIndex(aggregator, category))) {
     // Empty
}

/**
 * utility function to find the index corresponding to a category
 */
int categoryToIndex(const AbstractAggregator::Ptr aggregator, const QString& category) {
    int categoryIndex = -1;
    for(int i = 0 ; i < aggregator->categoryCount(); i++ ) {
        if(aggregator->categoryHeaderData(i,Qt::DisplayRole).toString() == category ) {
            categoryIndex = i;
            break;
        }
    }
    return categoryIndex;
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
