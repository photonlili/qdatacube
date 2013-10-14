#include "filter_by_aggregate.h"
#include "abstract_aggregator.h"

namespace qdatacube {

filter_by_aggregate_t::filter_by_aggregate_t(std::tr1::shared_ptr< abstract_aggregator_t > aggregator, int category_index, QObject* parent)
 : abstract_filter_t(aggregator->underlying_model(), parent),
   m_aggregator(aggregator),
   m_category_index(category_index)
{
  Q_ASSERT(aggregator);
  Q_ASSERT(category_index>=0);
  Q_ASSERT(category_index<aggregator->categoryCount());
  connect(aggregator.get(), SIGNAL(category_added(int)), SLOT(slot_aggregator_category_inserted(int)));
  connect(aggregator.get(), SIGNAL(category_removed(int)), SLOT(slot_aggregator_category_removed(int)));
}

filter_by_aggregate_t::filter_by_aggregate_t(qdatacube::abstract_aggregator_t* aggregator, int category_index, QObject* parent)
 : abstract_filter_t(aggregator->underlying_model(), parent),
   m_aggregator(aggregator),
   m_category_index(category_index)
{
  Q_ASSERT(aggregator);
  Q_ASSERT(category_index>=0);
  Q_ASSERT(category_index<aggregator->categoryCount());
  connect(aggregator, SIGNAL(category_added(int)), SLOT(slot_aggregator_category_inserted(int)));
  connect(aggregator, SIGNAL(category_removed(int)), SLOT(slot_aggregator_category_removed(int)));
}

QString filter_by_aggregate_t::name() const
{
  const static QString name = m_aggregator->name() + "=" + m_aggregator->categoryHeaderData(m_category_index).toString();;
  return name;
}

QString filter_by_aggregate_t::short_name() const
{
  return m_aggregator->categoryHeaderData(m_category_index).toString();
}

bool filter_by_aggregate_t::operator()(int row) {
  return ((*m_aggregator)(row) == m_category_index);
}

void filter_by_aggregate_t::slot_aggregator_category_inserted(int index) {
  if (index <= m_category_index) {
    ++m_category_index;
  }
}

void filter_by_aggregate_t::slot_aggregator_category_removed(int index) {
  if (index == m_category_index) {
    m_category_index = -1;
  } else if (index<m_category_index) {
    --m_category_index;
  }
}

}
#include "filter_by_aggregate.moc"
