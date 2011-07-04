#include "abstract_formatter.h"
#include "datacube_view.h"
#include <stdexcept>

namespace qdatacube {

abstract_formatter_t::abstract_formatter_t(QAbstractItemModel* underlying_model, datacube_view_t* view)
 : QObject(view),
   m_underlying_model(underlying_model),
   m_cell_size(10,10),
   m_datacube_view(view)
{
  if (!m_underlying_model) {
    throw std::runtime_error("underlying_model must be non-null");
  }
}

QSize abstract_formatter_t::cell_size() const
{
  return m_cell_size;
}

void abstract_formatter_t::set_cell_size(QSize size)
{
  if (m_cell_size != size) {
    m_cell_size = size;
    emit cell_size_changed(size);
  }
}

}

#include "abstract_formatter.moc"
