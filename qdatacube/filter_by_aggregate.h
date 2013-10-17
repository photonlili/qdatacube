#ifndef FILTER_BY_AGGREGATE_H
#define FILTER_BY_AGGREGATE_H

#include "abstract_filter.h"
#include "qdatacube_export.h"
#include <tr1/memory>

namespace qdatacube {

class AbstractAggregator;

class FilterByAggregatePrivate;
class QDATACUBE_EXPORT  FilterByAggregate : public AbstractFilter {
    Q_OBJECT
    public:
        FilterByAggregate(std::tr1::shared_ptr<AbstractAggregator> aggregator, int category_index);

        virtual bool operator()(int row) const;

        std::tr1::shared_ptr<AbstractAggregator> aggregator() const;

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
