#include "abstract_formatter.h"
#include "datacube_view.h"
#include <stdexcept>

namespace qdatacube {


class AbstractFormatterPrivate {
    public:
        AbstractFormatterPrivate(QAbstractItemModel* underlying_model, datacube_view_t* view) : m_underlying_model(underlying_model), m_view(view), m_cell_size(10,10) {

        }
        QAbstractItemModel* m_underlying_model;
        datacube_view_t* m_view;
        QSize m_cell_size;
};
abstract_formatter_t::abstract_formatter_t(QAbstractItemModel* underlying_model, datacube_view_t* view)
 : QObject(view), d(new AbstractFormatterPrivate(underlying_model,view))
{
    if (!underlying_model) {
        throw std::runtime_error("underlying_model must be non-null");
    }
}

QSize abstract_formatter_t::cell_size() const
{
  return d->m_cell_size;
}

void abstract_formatter_t::set_cell_size(QSize size)
{
  if (d->m_cell_size != size) {
    d->m_cell_size = size;
    emit cell_size_changed(size);
  }
}

datacube_view_t* abstract_formatter_t::datacubeView() const {
    return d->m_view;
}

QAbstractItemModel* abstract_formatter_t::underlyingModel() const {
    return d->m_underlying_model;
}

abstract_formatter_t::~abstract_formatter_t()
{

}




}

#include "abstract_formatter.moc"
