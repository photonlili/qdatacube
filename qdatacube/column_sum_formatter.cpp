#include "column_sum_formatter.h"
#include <QAbstractItemModel>
#include <QEvent>
#include <stdexcept>
#include <QWidget>
#include "datacube_view.h"
namespace qdatacube {

column_sum_formatter_t::column_sum_formatter_t(QAbstractItemModel* underlying_model, qdatacube::datacube_view_t* view, int column, int precision, QString suffix, double scale)
 : abstract_formatter_t(underlying_model, view),
   m_column(column),
   m_precision(precision),
   m_scale(scale),
   m_suffix(suffix)
{
  if (column >= underlying_model->columnCount()|| column<0) {
    throw std::runtime_error(QString("Column %1 must be in the underlying model, ie., be between 0 and %2").arg(column).arg(underlying_model->columnCount()).toStdString());
  }
  view->installEventFilter(this);
  recalculateCellSize();

}

QString column_sum_formatter_t::format(QList< int > rows) const
{
  double accumulator = 0;
  Q_FOREACH(int element, rows) {
    accumulator += m_underlying_model->index(element, m_column).data().toDouble();
  }
  return QString::number(accumulator*m_scale,'f',m_precision) + m_suffix;
}

QString column_sum_formatter_t::name() const
{
  return QString("Sum over %1").arg(m_underlying_model->headerData(m_column, Qt::Horizontal).toString());
}

QString column_sum_formatter_t::short_name() const
{
  return "SUM";
}

void column_sum_formatter_t::recalculateCellSize() {
  // Set the cell size, by summing up all the data in the model, and using that as input
  double accumulator = 0;
  for (int element = 0, nelements = m_underlying_model->rowCount(); element < nelements; ++element) {
    accumulator += m_underlying_model->index(element, m_column).data().toDouble();
  }
  QString big_cell_contents = QString::number(accumulator*m_scale, 'f', m_precision) + m_suffix;
  set_cell_size(QSize(m_datacube_view->fontMetrics().width(big_cell_contents), m_datacube_view->fontMetrics().lineSpacing()));
}

bool column_sum_formatter_t::eventFilter(QObject* object, QEvent* event) {
    if(object == m_datacube_view && event->type() == QEvent::FontChange) {
        recalculateCellSize();
    }
    return QObject::eventFilter(object, event);
}



} // end of namespace
