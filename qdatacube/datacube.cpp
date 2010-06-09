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
             shared_ptr<abstract_filter_t> column_filter);
    const QAbstractItemModel* model;
    QScopedPointer<datacube_colrow_t> columns;
    QScopedPointer<datacube_colrow_t> rows;
    std::tr1::shared_ptr<abstract_filter_t> global_filter;
    int global_filter_category;

};

datacube_t::secret_t::secret_t(const QAbstractItemModel* model,
                               shared_ptr<abstract_filter_t> row_filter,
                               shared_ptr<abstract_filter_t> column_filter) :
    model(model),
    columns(0L),
    rows(0L),
    global_filter(),
    global_filter_category(-1)
{
  QList<int> active;
  for (int row = 0, nrows = model->rowCount(); row<nrows; ++row) {
    active << row;
  }
  columns.reset(new datacube_colrow_t(model, column_filter, active));
  rows.reset(new datacube_colrow_t(model, row_filter, active));

}

datacube_t::datacube_t(const QAbstractItemModel* model,
                       shared_ptr<abstract_filter_t> row_filter,
                       shared_ptr<abstract_filter_t> column_filter,
                       QObject* parent):
    QObject(parent),
    d(new secret_t(model, row_filter, column_filter))
{
  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(update_data(QModelIndex,QModelIndex)));
  connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(remove_data(QModelIndex,int,int)));
  connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(insert_data(QModelIndex,int,int)));

}

datacube_t::datacube_t(const QAbstractItemModel* model,
                       abstract_filter_t* row_filter,
                       abstract_filter_t* column_filter,
                       QObject* parent):
    QObject(parent),
    d(new secret_t(model, shared_ptr<abstract_filter_t>(row_filter), shared_ptr<abstract_filter_t>(column_filter)))
{
  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(update_data(QModelIndex,QModelIndex)));
  connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(remove_data(QModelIndex,int,int)));
  connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(insert_data(QModelIndex,int,int)));
}

void datacube_t::set_global_filter(std::tr1::shared_ptr< qdatacube::abstract_filter_t > filter, int category) {
  for (int row = 0, nrows = d->model->rowCount(); row<nrows; ++row) {
    const bool was_included = d->global_filter.get() ? (*d->global_filter)(d->model, row) == d->global_filter_category : true;
    const bool to_be_included = filter.get() ? (*filter)(d->model, row) == category : true;
    if (!was_included && to_be_included) {
      add(row);
    } else if (was_included && !to_be_included) {
      remove(row);
    }
  }
  d->global_filter = filter;
  d->global_filter_category = category;
}

void datacube_t::set_global_filter(abstract_filter_t* filter, int category) {
  set_global_filter(std::tr1::shared_ptr<abstract_filter_t>(filter), category);
}

void datacube_t::reset_global_filter() {
  set_global_filter(std::tr1::shared_ptr<abstract_filter_t>(), -1);
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

void datacube_t::add(int index) {
  int row_section = d->rows->section_for_index(index);
  if (row_section == -1) {
    // Our datacube does not cover that container. Just ignore it.
    return;
  }
  int column_section = d->columns->section_for_index(index);
  Q_ASSERT(column_section>=0); // Every container should be in both rows and columns, or neither place.
  int row_to_add = -1;
  int column_to_add = -1;
  if(d->rows->sibling_indexes(index).isEmpty()) {
    row_to_add = row_section;
    emit row_about_to_be_added(row_section);
  }
  if(d->columns->sibling_indexes(index).isEmpty()) {
    column_to_add = column_section;
    emit column_about_to_be_added(column_section);
  }
  d->columns->add(index);
  d->rows->add(index);
  if(column_to_add>=0) {
    emit column_added(column_to_add);
  }
  if(row_to_add>=0) {
    emit row_added(row_to_add);
  }
  if(row_to_add==-1 && column_to_add==-1) {
    emit data_changed(row_section,column_section);
  }
}

void datacube_t::remove(int index) {
  int row_section = d->rows->section_for_index_internal(index);
  if (row_section == -1) {
    // Our datacube does not cover that container. Just ignore it.
    return;
  }
  int column_section = d->columns->section_for_index_internal(index);
  Q_ASSERT(column_section>=0); // Every container should be in both rows and columns, or neither place.
  int row_to_remove = -1;
  int column_to_remove = -1;
  if(d->rows->indexes(row_section).size()==1) {
    row_to_remove = row_section;
    emit row_about_to_be_removed(row_section);
  }
  if(d->columns->indexes(column_section).size()==1) {
    column_to_remove = column_section;
    emit column_about_to_be_removed(column_section);
  }
  d->columns->remove(index);
  d->rows->remove(index);
  if(column_to_remove>=0) {
    emit column_removed(column_to_remove);
  }
  if(row_to_remove>=0) {
    emit row_removed(row_to_remove);
  }
  if(row_to_remove==-1 && column_to_remove==-1) {
    emit data_changed(row_section,column_section);
  }
}

datacube_colrow_t& datacube_t::toplevel_row_header() {
  return *d->rows;
}
datacube_colrow_t& datacube_t::toplevel_column_header() {
  return *d->columns;
}

}

void qdatacube::datacube_t::update_data(QModelIndex topleft, QModelIndex bottomRight) {
  const int toprow = topleft.row();
  const int buttomrow = bottomRight.row();
  for (int row = toprow; row <= buttomrow; ++row) {
    const bool filtered_out = ((*d->global_filter)(d->model, row) != d->global_filter_category);
    const bool rowchanged = !d->rows->sibling_indexes(row).contains(row);
    const bool colchanged = !d->rows->sibling_indexes(row).contains(row);
    if (rowchanged || colchanged | filtered_out) {
      remove(row);
      if (!filtered_out) {
        add(row);
      }
    }
  }
}
void qdatacube::datacube_t::insert_data(QModelIndex parent, int start, int end) {
  Q_ASSERT(!parent.isValid());
  d->columns->adjust_before_add(end, end-start+1);
  d->rows->adjust_before_add(end, end-start+1);
  for (int row = start; row <=end; ++row) {
    add(row);
  }

}
void qdatacube::datacube_t::remove_data(QModelIndex parent, int start, int end) {
  Q_ASSERT(!parent.isValid());
  for (int row = end; row>=start; --row) {
    remove(row);
    d->columns->adjust_after_remove(row);
    d->rows->adjust_after_remove(row);
  }

}

#include "datacube.moc"

