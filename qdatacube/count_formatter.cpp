#include "count_formatter.h"
#include <QAbstractItemModel>
#include <QEvent>
#include "datacube_view.h"

namespace qdatacube {

count_formatter_t::count_formatter_t(QAbstractItemModel* underlying_model, qdatacube::datacube_view_t* view)
  : AbstractFormatter(underlying_model, view)
{
    update(qdatacube::AbstractFormatter::CellSize);
    setName(tr("Count"));
    setShortName("#");
}

QString count_formatter_t::format(QList< int > rows) const {
  return QString::number(rows.size());
}

void count_formatter_t::update(AbstractFormatter::UpdateType updateType) {
    if(updateType == qdatacube::AbstractFormatter::CellSize) {
        if(datacubeView()) {
        QString big_cell_contents = QString::number(underlyingModel()->rowCount());
            setCellSize(QSize(datacubeView()->fontMetrics().width(big_cell_contents),
                            datacubeView()->fontMetrics().ascent()+1));
        }
    }
}

}

