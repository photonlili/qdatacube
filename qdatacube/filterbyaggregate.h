#ifndef FILTER_BY_AGGREGATE_H
#define FILTER_BY_AGGREGATE_H

#include "abstractfilter.h"
#include "qdatacube_export.h"
#include "abstractaggregator.h"

namespace qdatacube {

class FilterByAggregatePrivate;
class QDATACUBE_EXPORT  FilterByAggregate : public AbstractFilter {
    Q_OBJECT
    public:
        FilterByAggregate(AbstractAggregator::Ptr aggregator, int category_index);

        virtual bool operator()(int row) const;

        AbstractAggregator::Ptr aggregator() const;

        int categoryIndex() const;
        virtual ~FilterByAggregate();
    private Q_SLOTS:
        void slot_aggregator_category_inserted(int index);
        void slot_aggregator_category_removed(int index);
    private:
        QScopedPointer<FilterByAggregatePrivate> d;
};

}
#endif // FILTER_BY_AGGREGATE_H
