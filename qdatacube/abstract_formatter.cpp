#include "abstract_formatter.h"
#include "datacube_view.h"
#include <stdexcept>
#include <QEvent>

namespace qdatacube {


class AbstractFormatterPrivate {
    public:
        AbstractFormatterPrivate(QAbstractItemModel* underlying_model, datacube_view_t* view) : m_underlying_model(underlying_model), m_view(view), m_cell_size(10,10) {
            m_name = "unnamed";
            m_shortName = "N/A";
        }
        QAbstractItemModel* m_underlying_model;
        datacube_view_t* m_view;
        QSize m_cell_size;
        QString m_name;
        QString m_shortName;
};
abstract_formatter_t::abstract_formatter_t(QAbstractItemModel* underlying_model, datacube_view_t* view)
 : QObject(view), d(new AbstractFormatterPrivate(underlying_model,view))
{
    if (!underlying_model) {
        throw std::runtime_error("underlying_model must be non-null");
    }
    if(view) {
        view->installEventFilter(this);
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

bool abstract_formatter_t::eventFilter(QObject* filter , QEvent* event ) {
    if(filter == datacubeView() && event->type() == QEvent::FontChange) {
        update(qdatacube::abstract_formatter_t::CellSize);
    }
    return QObject::eventFilter(filter, event);
}

void abstract_formatter_t::update(abstract_formatter_t::UpdateType element) {
    Q_UNUSED(element);
    // do nothing
}

QString abstract_formatter_t::name() const {
    return d->m_name;
}

void abstract_formatter_t::setName(const QString& newName) {
    d->m_name = newName;
}

void abstract_formatter_t::setShortName(const QString& newShortName) {
    d->m_shortName = newShortName;
}

QString abstract_formatter_t::short_name() const {
    return d->m_shortName;
}

abstract_formatter_t::~abstract_formatter_t()
{

}


}

#include "abstract_formatter.moc"
