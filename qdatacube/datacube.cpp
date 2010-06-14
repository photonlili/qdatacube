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

    /**
     * @returns the rows toplevel header
     */
    datacube_colrow_t& toplevel_row_header() ;

    /**
     * @returns the columns
     */
    datacube_colrow_t& toplevel_column_header() ;


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

datacube_colrow_t& datacube_t::secret_t::toplevel_row_header() {
  return *rows;
}
datacube_colrow_t& datacube_t::secret_t::toplevel_column_header() {
  return *columns;
}

void datacube_t::split_at_depth(Qt::Orientation orientation, const int start_section, datacube_colrow_t* parent, int depth, std::tr1::shared_ptr< abstract_filter_t > filter) {
  int section = start_section;
  if (depth == 0) {
    for (int bucket_no=0; bucket_no<parent->bucket_count(); ++bucket_no) {
      split_bucket(orientation, section, parent, bucket_no, filter);
      if (datacube_colrow_t* child =  parent->child_for_bucket(bucket_no)) {
        section += child->size();
      } else if (!parent->bucket_empty(bucket_no))  {
        ++section;
      }
    }
  } else {
    for (int bucket_no=0; bucket_no<parent->bucket_count(); ++bucket_no) {
      datacube_colrow_t* child = parent->child_for_bucket(bucket_no);
      if (child) {
        split_at_depth(orientation, section, child, depth-1, filter);
        section += child->size();
      } else {
        Q_ASSERT(false); // Untested.
        split_bucket(orientation, section, parent, bucket_no, filter);
        section += parent->child_for_bucket(bucket_no)->size();
      }
    }
  }

}

void datacube_t::collapse_at_depth(Qt::Orientation orientation, const int start_section, datacube_colrow_t* parent, int depth) {
  int section = start_section;
  if (depth==0) {
    for (int bucket_no=0; bucket_no<parent->bucket_count(); ++bucket_no) {
      collapse_bucket(orientation, section, parent, bucket_no);
      if (datacube_colrow_t* child =  parent->child_for_bucket(bucket_no)) {
        section += child->size();
      } else if (!parent->bucket_empty(bucket_no))  {
        ++section;
      }
    }
  } else {
    for (int bucket_no=0; bucket_no<parent->bucket_count(); ++bucket_no) {
      datacube_colrow_t* child = parent->child_for_bucket(bucket_no);
      if (child) {
        collapse_at_depth(orientation, section, child, depth-1);
        section += child->size();
      }
    }
  }
}

void datacube_t::split_bucket(Qt::Orientation orientation,  const int start_section, datacube_colrow_t* parent, int bucketno, std::tr1::shared_ptr< abstract_filter_t > filter) {
  QList<int> rows;
  datacube_colrow_t* oldchild = 0L;
  if (parent) {
    rows = parent->bucket_contents(bucketno);
    oldchild = parent->child_for_bucket(bucketno);
  } else {
    oldchild = (orientation==Qt::Horizontal ? d->columns : d->rows).data();
    for (int bucketno = 0, bucketcount = oldchild->bucket_count(); bucketno<bucketcount; ++bucketno) {
      rows << oldchild->bucket_contents(bucketno);
    }
    qSort(rows);
  }
  datacube_colrow_t* newchild = new datacube_colrow_t(d->model, filter, rows);
  int removecount = 0;
  if (oldchild) {
    for(int i=0; i<newchild->bucket_count(); ++i) {
      Q_ASSERT(newchild->child_for_bucket(i) == 0L);
      newchild->set_child(i, oldchild->deep_copy(newchild->bucket_contents(i)));
    }
    removecount = oldchild->size();
  } else {
    removecount = (parent->bucket_empty(bucketno))?0:1;
  }
  const int insertcount = newchild->size();
  const int count = insertcount-removecount;
  if (count<0) {
    if (orientation == Qt::Horizontal) {
      emit columns_about_to_be_removed(start_section, -count);
    } else {
      emit rows_about_to_be_removed(start_section, -count);
    }
  }
  if (count>0) {
    if (orientation == Qt::Horizontal) {
      emit columns_about_to_be_added(start_section, count);
    } else {
      emit rows_about_to_be_added(start_section, count);
    }
  }
  if (parent) {
    parent->set_child(bucketno, newchild);
  } else {
    if (orientation == Qt::Horizontal) {
      d->columns.reset(newchild);
    } else {
      d->rows.reset(newchild);
    }
  }
  if (count<0) {
    if (orientation == Qt::Horizontal) {
      emit columns_removed(start_section, -count);
    } else {
      emit rows_removed(start_section, -count);
    }
  }
  if (count>0) {
    if (orientation == Qt::Horizontal) {
      emit columns_added(start_section, count);
    } else {
      emit rows_added(start_section, count);
    }
  }
  if (removecount>0 && insertcount>0) {
    if (orientation == Qt::Horizontal) {
      slot_columns_changed(start_section, std::min(removecount, insertcount));
    } else {
      slot_rows_changed(start_section, std::min(removecount, insertcount));
    }
  }

}

void datacube_t::collapse_bucket(Qt::Orientation orientation, const int start_section, datacube_colrow_t* parent, int bucketno) {
  QList<int> rows;
  datacube_colrow_t* oldchild = 0L;
  if (parent) {
    rows = parent->bucket_contents(bucketno);
    oldchild = parent->child_for_bucket(bucketno);
  } else {
    oldchild = (orientation==Qt::Horizontal ? d->columns : d->rows).data();
    for (int bucketno = 0, bucketcount = oldchild->bucket_count(); bucketno<bucketcount; ++bucketno) {
      rows << oldchild->bucket_contents(bucketno);
    }
    qSort(rows);
  }
  const int removecount = oldchild->size();
  datacube_colrow_t* newchild = 0L;
  for (int cbucketno = 0, cbucketcount = oldchild->bucket_count(); cbucketno<cbucketcount; ++cbucketno) {
    if (datacube_colrow_t* grandchild = oldchild->child_for_bucket(cbucketno)) {
      newchild = grandchild->deep_copy(rows);
      break;
    }
  }
  const int insertcount = newchild ? newchild->size() : (rows.empty()?0:1);
  const int count = insertcount - removecount;

  // Emit about-to-signals
  if (count<0) {
    if (orientation == Qt::Horizontal) {
      emit columns_about_to_be_removed(start_section, -count);
    } else {
      emit rows_about_to_be_removed(start_section, -count);
    }
  }
  if (count>0) {
    if (orientation == Qt::Horizontal) {
      emit columns_about_to_be_added(start_section, count);
    } else {
      emit rows_about_to_be_added(start_section, count);
    }
  }

  //Actually collapse
  if (parent) {
    parent->set_child(bucketno, newchild);
  } else {
    if (orientation == Qt::Horizontal) {
      d->columns.reset(newchild);
    } else {
      d->rows.reset(newchild);
    }
  }

  // Emit done-signals
  if (count<0) {
    if (orientation == Qt::Horizontal) {
      emit columns_removed(start_section, -count);
    } else {
      emit rows_removed(start_section, -count);
    }
  }
  if (count>0) {
    if (orientation == Qt::Horizontal) {
      emit columns_added(start_section, count);
    } else {
      emit rows_added(start_section, count);
    }
  }
  if (removecount>0 && insertcount>0) {
    if (orientation == Qt::Horizontal) {
      slot_columns_changed(start_section, std::min(removecount, insertcount));
    } else {
      slot_rows_changed(start_section, std::min(removecount, insertcount));
    }
  }

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
    emit rows_about_to_be_added(row_section,1);
  }
  if(d->columns->sibling_indexes(index).isEmpty()) {
    column_to_add = column_section;
    emit columns_about_to_be_added(column_section,1);
  }
  d->columns->add(index);
  d->rows->add(index);
  if(column_to_add>=0) {
    emit columns_added(column_to_add,1);
  }
  if(row_to_add>=0) {
    emit rows_added(row_to_add,1);
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
    emit rows_about_to_be_removed(row_section,1);
  }
  if(d->columns->indexes(column_section).size()==1) {
    column_to_remove = column_section;
    emit columns_about_to_be_removed(column_section,1);
  }
  d->columns->remove(index);
  d->rows->remove(index);
  if(column_to_remove>=0) {
    emit columns_removed(column_to_remove,1);
  }
  if(row_to_remove>=0) {
    emit rows_removed(row_to_remove,1);
  }
  if(row_to_remove==-1 && column_to_remove==-1) {
    emit data_changed(row_section,column_section);
  }
}

void datacube_t::update_data(QModelIndex topleft, QModelIndex bottomRight) {
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
void datacube_t::insert_data(QModelIndex parent, int start, int end) {
  Q_ASSERT(!parent.isValid());
  d->columns->adjust_before_add(end, end-start+1);
  d->rows->adjust_before_add(end, end-start+1);
  for (int row = start; row <=end; ++row) {
    add(row);
  }

}
void datacube_t::remove_data(QModelIndex parent, int start, int end) {
  Q_ASSERT(!parent.isValid());
  for (int row = end; row>=start; --row) {
    remove(row);
    d->columns->adjust_after_remove(row);
    d->rows->adjust_after_remove(row);
  }

}

void datacube_t::slot_columns_changed(int column, int count) {
  for (int col = column; col<=column+count;++col) {
    const int rowcount = rowCount();
    for (int row = 0; row < rowcount; ++row) {
      emit data_changed(row,col);
    }
  }
  emit headers_changed(Qt::Horizontal, column, column+count-1);
}

void datacube_t::slot_rows_changed(int row, int count) {
  for (int r = row; r<=row+count;++r) {
    const int columncount = columnCount();
    for (int column = 0; column < columncount; ++column) {
      emit data_changed(r,column);
    }
  }
  emit headers_changed(Qt::Vertical, row, row+count-1);
}

void datacube_t::split(Qt::Orientation orientation, int headerno, std::tr1::shared_ptr< abstract_filter_t > filter) {
  if (headerno==0) {
    split_bucket(orientation, 0, 0L, -1, filter);
  }
  datacube_colrow_t* colrow = &(orientation == Qt::Horizontal ? d->toplevel_column_header() : d->toplevel_row_header());
  if (headerno>=1) {
    split_at_depth(orientation, 0, colrow, headerno-1, filter);
  }
}

void datacube_t::split(Qt::Orientation orientation, int headerno, abstract_filter_t* filter) {
  split(orientation, headerno, std::tr1::shared_ptr<abstract_filter_t>(filter));
}


void datacube_t::collapse(Qt::Orientation orientation, int headerno) {
  if (headerCount(orientation)<2) {
    return;
  }
  if (headerno>0) {
    datacube_colrow_t* parent = (Qt::Horizontal==orientation ? d->columns : d->rows).data();
    collapse_at_depth(orientation, 0, parent, headerno-1);
  } else {
    collapse_bucket(orientation, 0, 0L, -1);
  }
}

QList< std::tr1::shared_ptr< abstract_filter_t > > datacube_t::filters_for_section(Qt::Orientation orientation, int section) const {
  if (orientation == Qt::Horizontal) {
    return d->columns->filters_for_section(section);
  } else {
    return d->rows->filters_for_section(section);
  }
}


}


#include "datacube.moc"

