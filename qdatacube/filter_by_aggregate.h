#ifndef FILTER_BY_AGGREGATE_H
#define FILTER_BY_AGGREGATE_H

#include "abstract_filter.h"
#include "qdatacube_export.h"
#include <tr1/memory>

namespace qdatacube {

class abstract_aggregator_t;

class FilterByAggregatePrivate;
class QDATACUBE_EXPORT  filter_by_aggregate_t : public abstract_filter_t {
  Q_OBJECT
  public:
    filter_by_aggregate_t(std::tr1::shared_ptr<abstract_aggregator_t> aggregator, int category_index, QObject* parent = 0);

    virtual bool operator()(int row) const;

    std::tr1::shared_ptr<abstract_aggregator_t> aggregator() const;

    int category_index() const;
    virtual ~filter_by_aggregate_t();
  private Q_SLOTS:
    void slot_aggregator_category_inserted(int index);
    void slot_aggregator_category_removed(int index);
  private:
      QScopedPointer<FilterByAggregatePrivate> d;
};

}
#endif // FILTER_BY_AGGREGATE_H
