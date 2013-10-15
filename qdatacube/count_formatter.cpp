#include "count_formatter.h"
#include <QAbstractItemModel>
#include <QEvent>
#include "datacube_view.h"

namespace qdatacube {

count_formatter_t::count_formatter_t(QAbstractItemModel* underlying_model, qdatacube::datacube_view_t* view)
  : abstract_formatter_t(underlying_model, view)
{
    update(qdatacube::abstract_formatter_t::CellSize);
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

void count_formatter_t::update(abstract_formatter_t::UpdateType updateType) {
    if(updateType == qdatacube::abstract_formatter_t::CellSize) {
        if(datacubeView()) {
        QString big_cell_contents = QString::number(underlyingModel()->rowCount());
            set_cell_size(QSize(datacubeView()->fontMetrics().width(big_cell_contents),
                            datacubeView()->fontMetrics().ascent()+1));
        }
    }
}

}

