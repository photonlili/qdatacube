#include "count_formatter.h"
#include <QAbstractItemModel>
#include "datacube_view.h"

namespace qdatacube {

count_formatter_t::count_formatter_t(QAbstractItemModel* underlying_model, qdatacube::datacube_view_t* view)
  : abstract_formatter_t(underlying_model, view)
{
  QString big_cell_contents = QString::number(m_underlying_model->rowCount());
  set_cell_size(QSize(view->fontMetrics().width(big_cell_contents), view->fontMetrics().ascent()+1));

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
}

