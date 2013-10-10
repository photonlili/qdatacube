#include "count_formatter.h"
#include <QAbstractItemModel>
#include <QEvent>
#include "datacube_view.h"

namespace qdatacube {

count_formatter_t::count_formatter_t(QAbstractItemModel* underlying_model, qdatacube::datacube_view_t* view)
  : abstract_formatter_t(underlying_model, view)
{
    recalculateCellSize();
    if(view) {
        view->installEventFilter(this);
    }
}

QString count_formatter_t::name() const {
  return tr("Count");
}

QString count_formatter_t::short_name() const {
  return tr("#");

}

QString count_formatter_t::format(QList< int > rows) const {
  return QString::number(rows.size());
}

bool count_formatter_t::eventFilter(QObject* filter, QEvent* event) {
    if(filter == m_datacube_view && event->type() == QEvent::FontChange) {
        recalculateCellSize();
    }
    return QObject::eventFilter(filter, event);
}

void count_formatter_t::recalculateCellSize() {
    if(m_datacube_view) {
    QString big_cell_contents = QString::number(m_underlying_model->rowCount());
        set_cell_size(QSize(m_datacube_view->fontMetrics().width(big_cell_contents),
                            m_datacube_view->fontMetrics().ascent()+1));
    }
}
}

