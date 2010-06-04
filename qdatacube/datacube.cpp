/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube.h"
#include "datacube_colrow.h"
#include "abstract_filter.h"


#include <QAbstractItemModel>
using std::tr1::shared_ptr;

namespace qdatacube {

class datacube_t::secret_t {
  public:
    secret_t(const QAbstractItemModel* model,
             shared_ptr<abstract_filter_t> row_filter,
             shared_ptr<abstract_filter_t> column_filter,
             const QList<int>& active);
    QScopedPointer<datacube_colrow_t> columns;
    QScopedPointer<datacube_colrow_t> rows;

};

datacube_t::secret_t::secret_t(const QAbstractItemModel* model,
                               shared_ptr<abstract_filter_t> row_filter,
                               shared_ptr<abstract_filter_t> column_filter,
                               const QList< int >& active) :
    columns(new datacube_colrow_t(model, column_filter, active)),
    rows(new datacube_colrow_t(model, row_filter, active))
{

}

datacube_t::datacube_t(const QAbstractItemModel* model,
                       shared_ptr<abstract_filter_t> row_filter,
                       shared_ptr<abstract_filter_t> column_filter,
                       const QList<int>& active,
                       QObject* parent):
    QObject(parent),
    d(new secret_t(model, row_filter, column_filter, active))
{

}

datacube_t::datacube_t(const QAbstractItemModel* model,
                       abstract_filter_t* row_filter,
                       abstract_filter_t* column_filter,
                       const QList< int >& active,
                       QObject* parent):
    QObject(parent),
    d(new secret_t(model, shared_ptr<abstract_filter_t>(row_filter), shared_ptr<abstract_filter_t>(column_filter), active))
{
}

int datacube_t::headerCount(Qt::Orientation orientation) const {
  return orientation == Qt::Horizontal ? d->columns->depth() : d->rows->depth();
}

int datacube_t::cellCount(int row, int column) const {
  return cellrows(row,column).size();
}

int datacube_t::columnCount() const {
  return d->columns->size();
}

int datacube_t::rowCount() const {
  return d->rows->size();
}

QList< int > datacube_t::cellrows(int row, int column) const {
  // Return the intersection of the row's and the column's container indexes
  QList<int> rowindex = d->rows->indexes(row);
  QList<int> colindex = d->columns->indexes(column);
  int r = 0;
  int c = 0;
  QList<int> rv;
  while (r < rowindex.size() && c < colindex.size()) {
    if (rowindex[r] == colindex[c]) {
      rv << colindex[c];
      ++r;
      ++c;
    } else if (rowindex[r] < colindex[c]) {
      ++r;
    } else if (rowindex[r] > colindex[c]) {
      ++c;
    }

  }
  return rv;

}

QList< QPair<QString,int> > datacube_t::headers(Qt::Orientation orientation, int index) const {
  if (orientation == Qt::Horizontal) {
    return d->columns->active_headers(index);
  } else {
    return d->rows->active_headers(index);
  }
}

datacube_t::~datacube_t() {
  // Need to declare here so datacube_colrow_t's destructor is visible
}

int datacube_t::depth(Qt::Orientation orientation) {
  return orientation == Qt::Horizontal ? d->columns->depth() : d->rows->depth();
}

void datacube_t::restrict(QList< int > set) {
  d->columns->restrict(set);
  d->rows->restrict(set);
  emit changed();
}

void datacube_t::remove(int container_index) {
  int row_index = d->rows->bucket_for_index(container_index);
  if (row_index == -1) {
    // Our qdatacube does not cover that container. Just ignore it.
    return;
  }
  int column_index = d->columns->bucket_for_index(container_index);
  Q_ASSERT(column_index>=0); // Every container should be in both rows and columns, or neither place.
  int row_to_remove = -1;
  int column_to_remove = -1;
  if(d->rows->indexes(row_index).size()==1) {
    row_to_remove = row_index;
    emit row_about_to_be_removed(row_index);
  }
  if(d->columns->indexes(column_index).size()==1) {
    column_to_remove = column_index;
    emit column_about_to_be_removed(column_index);
  }
  d->columns->remove(container_index);
  d->rows->remove(container_index);
  if(column_to_remove>=0) {
    emit column_removed(column_to_remove);
  }
  if(row_to_remove>=0) {
    emit row_removed(row_to_remove);
  }
  if(row_to_remove==-1 && column_to_remove==-1) {
    emit data_changed(row_index,column_index);
  }
}

void datacube_t::readd(int index) {
  d->columns->readd(index);
  d->rows->readd(index);
  emit changed();
}
datacube_colrow_t& datacube_t::toplevel_row_header() {
  return *d->rows;
}
datacube_colrow_t& datacube_t::toplevel_column_header() {
  return *d->columns;
}

}

#include "datacube.moc"

