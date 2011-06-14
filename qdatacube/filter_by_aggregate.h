#ifndef FILTER_BY_AGGREGATE_H
#define FILTER_BY_AGGREGATE_H

#include "abstract_filter.h"
#include "qdatacube_export.h"
#include <tr1/memory>

namespace qdatacube {

class abstract_aggregator_t;


class QDATACUBE_EXPORT  filter_by_aggregate_t : public abstract_filter_t {
  Q_OBJECT
  public:
    filter_by_aggregate_t(std::tr1::shared_ptr<abstract_aggregator_t> aggregator, int category_index, QObject* parent = 0);
    filter_by_aggregate_t(abstract_aggregator_t* aggregator, int category_index, QObject* parent = 0);

    virtual QString name() const;

    virtual QString short_name() const;

    virtual bool operator()(int row);

    std::tr1::shared_ptr<abstract_aggregator_t> aggregator() const {
      return m_aggregator;
    }

    int category_index() const {
      return m_category_index;
    }
  private Q_SLOTS:
    void slot_aggregator_category_inserted(int index);
    void slot_aggregator_category_removed(int index);
  private:
    std::tr1::shared_ptr<abstract_aggregator_t> m_aggregator;
    int m_category_index;
};

}
#endif // FILTER_BY_AGGREGATE_H
