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
        /**
         * Creates a filter to filter in a given category in the aggregator.
         */
        FilterByAggregate(AbstractAggregator::Ptr aggregator, int category_index);
        /**
         * Creates a filter to filter in a given category in the aggregator.
         * The category is found by comparing the display text for the various categories
         * in the aggregator and uses the first match found.
         *
         * To query if something *is* found, you can check that \ref categoryIndex() is non-negative
         */
        FilterByAggregate(AbstractAggregator::Ptr aggregator, const QString& categoryLabel);

        virtual bool operator()(int row) const;

        AbstractAggregator::Ptr aggregator() const;

        int categoryIndex() const;
        virtual ~FilterByAggregate();
    private Q_SLOTS:
        void slot_aggregator_category_inserted(int index);
        void slot_aggregator_category_removed(int index);
    private:
        QScopedPointer<FilterByAggregatePrivate> d;
        friend FilterByAggregatePrivate;
};

}
#endif // FILTER_BY_AGGREGATE_H
