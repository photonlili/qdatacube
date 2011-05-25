/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

// Enable this to get a lot of consistency checks
// #define ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS 1

#include "datacube.h"
#include "abstract_filter.h"

#include <QVector>
#include <algorithm>

#include <QAbstractItemModel>
#include "cell.h"
#include "datacube_selection.h"
using std::tr1::shared_ptr;

namespace qdatacube {

class datacube_t::secret_t {
  public:
    secret_t(const QAbstractItemModel* model,
             shared_ptr<abstract_filter_t> row_filter,
             shared_ptr<abstract_filter_t> column_filter);
    int compute_row_section_for_index(int index) {
      return compute_section_for_index(Qt::Vertical, index);
    }
    int compute_column_section_for_index(int index) {
      return compute_section_for_index(Qt::Horizontal, index);
    }
    int compute_section_for_index(Qt::Orientation orientation, int index);
    QList<int>& cell(int bucket_row, int bucket_column);
    int bucket_to_row(int bucket_row);
    int bucket_to_column(int bucket_column);
    int bucket_for_row(int row);
    int bucket_for_column(int column);
    const QAbstractItemModel* model;
    typedef QVector<shared_ptr<abstract_filter_t> >filters_t;
    filters_t row_filters;
    filters_t col_filters;
    QVector<unsigned> row_counts;
    QVector<unsigned> col_counts;
    QVector<QList<int> > cells;
    std::tr1::shared_ptr<abstract_filter_t> global_filter;
    typedef QHash<int, cell_t> reverse_index_t;
    reverse_index_t reverse_index;
    int global_filter_category;
    QList<datacube_selection_t*> selection_models;
};

int datacube_t::secret_t::compute_section_for_index(Qt::Orientation orientation, int index) {
  qdatacube::datacube_t::secret_t::filters_t& filters = orientation == Qt::Horizontal ? col_filters : row_filters;
  int stride = 1;
  int rv = 0;
  for (int filter_index = filters.size()-1; filter_index>=0; --filter_index) {
    const shared_ptr<abstract_filter_t>& filter = filters.at(filter_index);
    rv += stride * (*filter)(model, index);
    stride *= filter->categories(model).size();
  }
  return rv;
}

int datacube_t::secret_t::bucket_for_row(const int row) {
  int r = row;
  int section=0;
  for (; section<row_counts.size(); ++section) {
    if (row_counts[section]>0) {
      if (r-- == 0) {
        return section;
      }
    }
  }
  Q_ASSERT_X(false, "QDatacube", QString("Row %1 too big for qdatacube with %2 rows").arg(row).arg(row_counts.size() - std::count(row_counts.begin(), row_counts.end(), 0u)).toLocal8Bit().data());
  return -1;
}

int datacube_t::secret_t::bucket_for_column(int column) {
  int section=0;
  int c = column;
  for (; section<col_counts.size(); ++section) {
    if (col_counts[section]>0) {
      if (c-- == 0) {
        return section;
      }
    }
  }
  Q_ASSERT_X(false, "qdatacube", QString("Column %1 too big for qdatacube with %2 columns").arg(column).arg(col_counts.size() - std::count(col_counts.begin(), col_counts.end(), 0u)).toLocal8Bit().data());
  return -1;
}

QList<int>& datacube_t::secret_t::cell(int bucket_row, int bucket_column) {
  const int i = bucket_row + bucket_column*row_counts.size();
  return cells[i];

}

int datacube_t::secret_t::bucket_to_column(int bucket_column) {
  int rv = 0;
  for (int i=0; i<bucket_column; ++i) {
    if (col_counts[i]>0) {
      ++rv;
    }
  }
  return rv;

}

int datacube_t::secret_t::bucket_to_row(int bucket_row) {
  int rv = 0;
  for (int i=0; i<bucket_row; ++i) {
    if (row_counts[i]>0) {
      ++rv;
    }
  }
  return rv;

}

datacube_t::secret_t::secret_t(const QAbstractItemModel* model,
                               shared_ptr<abstract_filter_t> row_filter,
                               shared_ptr<abstract_filter_t> column_filter) :
    model(model),
    global_filter(),
    global_filter_category(-1)
{
  col_filters << column_filter;
  row_filters << row_filter;
  col_counts = QVector<unsigned>(column_filter->categories(model).size());
  row_counts = QVector<unsigned>(row_filter->categories(model).size());
  cells = QVector<QList<int> >(col_counts.size() * row_counts.size());
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
  connect(column_filter.get(), SIGNAL(category_added(int)), SLOT(slot_filter_category_added(int)));
  connect(row_filter.get(), SIGNAL(category_added(int)), SLOT(slot_filter_category_added(int)));
  connect(column_filter.get(), SIGNAL(category_removed(int)), SLOT(slot_filter_category_removed(int)));
  connect(row_filter.get(), SIGNAL(category_removed(int)), SLOT(slot_filter_category_removed(int)));;
  for (int element = 0, nelements = model->rowCount(); element < nelements; ++element) {
    add(element);
  }
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif

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
  connect(column_filter, SIGNAL(category_added(int)), SLOT(slot_filter_category_added(int)));
  connect(row_filter, SIGNAL(category_added(int)), SLOT(slot_filter_category_added(int)));
  connect(column_filter, SIGNAL(category_removed(int)), SLOT(slot_filter_category_removed(int)));
  connect(row_filter, SIGNAL(category_removed(int)), SLOT(slot_filter_category_removed(int)));;
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
}

void datacube_t::set_global_filter(std::tr1::shared_ptr< qdatacube::abstract_filter_t > filter, int category) {
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
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
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
}

void datacube_t::check() {
  if (d->row_counts.size() * d->col_counts.size() != d->cells.size()) {
    qDebug() << row_count() << "*" << column_count() <<  "!=" << d->cells.size();
    Q_ASSERT(row_count() * column_count() == d->cells.size());
  }
  int total_count = 0;
  const int nelements = d->model->rowCount();
  for (int i=0; i<nelements;  ++i) {
    if (!d->global_filter.get() || (*d->global_filter)(d->model, i) == d->global_filter_category) {
      ++total_count;
    }
  }
  QVector<unsigned> check_row_counts(d->row_counts.size());
  int count = 0;
  for (int c=0; c<d->col_counts.size(); ++c) {
    unsigned col_count = 0;
    for (int r=0; r<d->row_counts.size(); ++r) {
      const int nelements = d->cell(r,c).size();
      check_row_counts[r] += nelements;
      col_count += nelements;
    }
    if (col_count != d->col_counts[c]) {
      qDebug() << col_count << "!=" << d->col_counts[c];
      Q_ASSERT(col_count == d->col_counts[c]);
    }
    count += col_count;
  }
  Q_ASSERT(count == total_count);
  for (int i=0; i<d->row_counts.size(); ++i) {
    Q_ASSERT(check_row_counts[i] == d->row_counts[i]);
  }

}

void datacube_t::set_global_filter(abstract_filter_t* filter, int category) {
  set_global_filter(std::tr1::shared_ptr<abstract_filter_t>(filter), category);
}

void datacube_t::reset_global_filter() {
  set_global_filter(std::tr1::shared_ptr<abstract_filter_t>(), -1);
}

int datacube_t::header_count(Qt::Orientation orientation) const {
  return orientation == Qt::Horizontal ? d->col_filters.size() : d->row_filters.size();
}

int datacube_t::element_count(int row, int column) const {
  return  elements(row,column).size();
}

int datacube_t::column_count() const {
  return d->col_counts.size() - std::count(d->col_counts.begin(), d->col_counts.end(), 0u);
}

int datacube_t::row_count() const {
  return d->row_counts.size() - std::count(d->row_counts.begin(), d->row_counts.end(), 0u);
}

QList< int > datacube_t::elements(int row, int column) const {
  // Note that this function should be very fast indeed.
  const int row_section = d->bucket_for_row(row);
  const int col_section = d->bucket_for_column(column);
  return d->cell(row_section, col_section);

}

QList< QPair<QString,int> > datacube_t::headers(Qt::Orientation orientation, int index) const {
  QList< QPair<QString,int> > rv;
  QVector<shared_ptr<abstract_filter_t> >& filters = (orientation == Qt::Horizontal) ? d->col_filters : d->row_filters;
  const QVector<unsigned>& counts = (orientation == Qt::Horizontal) ? d->col_counts : d->row_counts;
  shared_ptr<abstract_filter_t> filter = filters.at(index);
  QList<QString> categories = filter->categories(d->model);
  const int ncats = categories.size();
  int stride = 1;
  for (int i=index+1; i<filters.size(); ++i) {
    stride *= filters.at(i)->categories(d->model).size();
  }
  for (int c=0; c<counts.size(); c+=stride) {
    int count = 0;
    for (int i=0;i<stride; ++i) {
      if (counts.at(c+i)>0) {
        ++count;
      }
    }
    if (count > 0 ) {
      rv << QPair<QString,int>(categories.at((c/stride)%ncats), count);
    }
  }
  return rv;
}

datacube_t::~datacube_t() {
  // Need to declare here so datacube_colrow_t's destructor is visible
}

void datacube_t::add(int index) {

  // Computer bucket
  int row_section = d->compute_row_section_for_index(index);
  if (row_section == -1) {
    // Our datacube does not cover that container. Just ignore it.
    return;
  }
  int column_section = d->compute_column_section_for_index(index);
  Q_ASSERT(column_section>=0); // Every container should be in both rows and columns, or neither place.

  // Check if rows/columns are added, and notify listernes as neccessary
  int row_to_add = -1;
  int column_to_add = -1;
  if(d->row_counts[row_section]++ == 0) {
    row_to_add = d->bucket_to_row(row_section);;
    emit rows_about_to_be_inserted(row_to_add,1);
  }
  if(d->col_counts[column_section]++ == 0) {
    column_to_add = d->bucket_to_column(column_section);
    emit columns_about_to_be_inserted(column_to_add,1);
  }

  // Actually add
  d->cell(row_section, column_section) << index;
  d->reverse_index.insert(index, cell_t(row_section, column_section));

  // Notify various listerners
  Q_FOREACH(datacube_selection_t* selection, d->selection_models) {
    selection->datacube_adds_element_to_bucket(row_section, column_section, index);
  }
  if(column_to_add>=0) {
    emit columns_inserted(column_to_add,1);
  }
  if(row_to_add>=0) {
    emit rows_inserted(row_to_add,1);
  }
  if(row_to_add==-1 && column_to_add==-1) {
    emit data_changed(d->bucket_to_row(row_section),d->bucket_to_column(column_section));
  }
}

void datacube_t::remove(int index) {
  cell_t cell = d->reverse_index.value(index);
  if (cell.invalid()) {
    // Our datacube does not cover that container. Just ignore it.
    return;
  }
  Q_FOREACH(datacube_selection_t* selection, d->selection_models) {
    selection->datacube_removes_element_from_bucket(cell.row(), cell.column(), index);
  }
  int row_to_remove = -1;
  int column_to_remove = -1;
  if(--d->row_counts[cell.row()]==0) {
    row_to_remove = d->bucket_to_row(cell.row());
    emit rows_about_to_be_removed(row_to_remove,1);
  }
  if(--d->col_counts[cell.column()]==0) {
    column_to_remove = d->bucket_to_column(cell.column());
    emit columns_about_to_be_removed(column_to_remove,1);
  }
  const bool check = d->cell(cell.row(), cell.column()).removeOne(index);
  Q_UNUSED(check)
  Q_ASSERT(check);
  d->reverse_index.remove(index);
  if(column_to_remove>=0) {
    emit columns_removed(column_to_remove,1);
  }
  if(row_to_remove>=0) {
    emit rows_removed(row_to_remove,1);
  }
  if(row_to_remove==-1 && column_to_remove==-1) {
    emit data_changed(d->bucket_to_row(cell.row()),d->bucket_to_column(cell.column()));
  }
}

void datacube_t::update_data(QModelIndex topleft, QModelIndex bottomRight) {
  const int toprow = topleft.row();
  const int buttomrow = bottomRight.row();
  for (int element = toprow; element <= buttomrow; ++element) {
    const bool filtered_out = d->global_filter.get() && ((*d->global_filter)(d->model, element) != d->global_filter_category);
    int new_row_section = d->compute_section_for_index(Qt::Vertical, element);
    int new_column_section = d->compute_section_for_index(Qt::Horizontal, element);
    cell_t old_cell = d->reverse_index.value(element);
    const bool rowchanged = old_cell.row() != new_row_section;
    const bool colchanged = old_cell.column() != new_column_section;
    if (rowchanged || colchanged | filtered_out) {
      remove(element);
      if (!filtered_out) {
        add(element);
      }
    }
  }
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
}

void datacube_t::insert_data(QModelIndex parent, int start, int end) {
  Q_ASSERT(!parent.isValid());
  for (int row = start; row <=end; ++row) {
    if(!d->global_filter.get() || (*d->global_filter)(d->model,row)==d->global_filter_category) {
      add(row);
    }
  }
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif

}

void datacube_t::remove_data(QModelIndex parent, int start, int end) {
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
  Q_ASSERT(!parent.isValid());
  for (int row = end; row>=start; --row) {
    remove(row);
  }

}

void datacube_t::slot_columns_changed(int column, int count) {
  for (int col = column; col<=column+count;++col) {
    const int rowcount = row_count();
    for (int row = 0; row < rowcount; ++row) {
      emit data_changed(row,col);
    }
  }
  emit headers_changed(Qt::Horizontal, column, column+count-1);
}

void datacube_t::slot_rows_changed(int row, int count) {
  for (int r = row; r<=row+count;++r) {
    const int columncount = column_count();
    for (int column = 0; column < columncount; ++column) {
      emit data_changed(r,column);
    }
  }
  emit headers_changed(Qt::Vertical, row, row+count-1);
}

void datacube_t::split(Qt::Orientation orientation, int headerno, std::tr1::shared_ptr< abstract_filter_t > filter) {
  emit about_to_be_reset();
  if (orientation == Qt::Vertical) {
    split_row(headerno, filter);
  } else {
    split_column(headerno, filter);
  }
  if (!d->row_filters.contains(filter) && d->col_filters.contains(filter)) {
    connect(filter.get(), SIGNAL(category_added(int)), SLOT(slot_filter_category_added(int)));
    connect(filter.get(), SIGNAL(category_removed(int)), SLOT(slot_filter_category_removed(int)));;
  }
  emit reset();
}

void datacube_t::split_row(int headerno, std::tr1::shared_ptr< abstract_filter_t > filter)
{
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
  emit about_to_be_reset();
  QVector<QList<int> > oldcells = d->cells;
  const int ncats = filter->categories(d->model).size();
  const int newsize = oldcells.size() * ncats;
  d->cells = QVector<QList<int> >(newsize);
  const int source_row_count = d->row_counts.size();
  const int target_row_count = source_row_count * ncats;
  int cat_stride = 1;
  for (int i=headerno; i<d->row_filters.size(); ++i) {
    cat_stride *= d->row_filters.at(i)->categories(d->model).size();
  }
  int target_stride = cat_stride*ncats;
  QVector<unsigned> old_row_counts = d->row_counts;
  d->row_counts = QVector<unsigned>(old_row_counts.size()*ncats);
  d->reverse_index = secret_t::reverse_index_t();
  // Sort out elements in new categories. Note that the old d->col_counts are unchanged
  for (int c=0; c<d->col_counts.size(); ++c) {
    for (int major = 0; major<old_row_counts.size()/cat_stride; ++major) {
      for (int minor = 0; minor<cat_stride; ++minor) {
        const int r = major*cat_stride + minor;
        Q_FOREACH(int element, oldcells[source_row_count*c+r]) {
          const int target_row = major*target_stride + minor + (*filter).operator()(d->model, element) * cat_stride;
          const int target_index = target_row + c*target_row_count;
          d->cells[target_index] << element;
          ++d->row_counts[target_row];
          d->reverse_index.insert(element, cell_t(target_row, c));
        }
      }
    }
  }
  d->row_filters.insert(headerno, filter);
  emit reset();
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif

}

void datacube_t::split_column(int headerno, std::tr1::shared_ptr< abstract_filter_t > filter) {
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
  emit about_to_be_reset();
  QVector<QList<int> > oldcells = d->cells;
  const int ncats = filter->categories(d->model).size();
  const int newsize = oldcells.size() * ncats;
  d->cells = QVector<QList<int> >(newsize);
  const int row_count = d->row_counts.size();
  int cat_stride = 1;
  for (int i=headerno; i<d->col_filters.size(); ++i) {
    cat_stride *= d->col_filters.at(i)->categories(d->model).size();
  }
  int target_stride = cat_stride*ncats;
  QVector<unsigned> old_column_counts = d->col_counts;
  d->col_counts = QVector<unsigned>(old_column_counts.size()*ncats);
  d->reverse_index = secret_t::reverse_index_t();
  // Sort out elements in new categories. Note that the old d->row_counts are unchanged
  for (int major = 0; major < old_column_counts.size()/cat_stride; ++major) {
    for (int minor = 0; minor < cat_stride; ++minor) {
      const int c = major*cat_stride + minor;
      for (int r=0; r<d->row_counts.size(); ++r) {
        Q_FOREACH(int element, oldcells[row_count*c+r]) {
          const int target_column = major*target_stride + minor + (*filter).operator()(d->model, element) * cat_stride;
          const int target_index = r + target_column*d->row_counts.size();
          d->cells[target_index] << element;
          d->reverse_index.insert(element, cell_t(r, target_column));
          ++d->col_counts[target_column];
        }
      }
    }
  }
  d->col_filters.insert(headerno, filter);
  emit reset();
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif

}


void datacube_t::split(Qt::Orientation orientation, int headerno, abstract_filter_t* filter) {
  split(orientation, headerno, std::tr1::shared_ptr<abstract_filter_t>(filter));
}


void datacube_t::collapse(Qt::Orientation orientation, int headerno) {
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
  emit about_to_be_reset();
  QVector<QList<int> > oldcells = d->cells;
  const bool horizontal = (orientation == Qt::Horizontal);
  secret_t::filters_t& parallel_filters = horizontal ? d->col_filters : d->row_filters;
  shared_ptr<abstract_filter_t> filter = parallel_filters[headerno];
  parallel_filters.remove(headerno);
  const int ncats = filter->categories(d->model).size();
  const int newsize = oldcells.size() / ncats;
  d->cells = QVector<QList<int> >(newsize);
  const int normal_count = horizontal ? d->row_counts.size() : d->col_counts.size();
  int cat_stride = 1;
  for (int i=headerno; i<parallel_filters.size(); ++i) {
    cat_stride *= parallel_filters.at(i)->categories(d->model).size();
  }
  const int source_stride = cat_stride * ncats;
  QVector<unsigned>& new_counts = horizontal ? d->col_counts : d->row_counts;
  QVector<unsigned> old_counts = new_counts;
  new_counts = QVector<unsigned>(old_counts.size()/ncats);
  d->reverse_index = secret_t::reverse_index_t();
  for (int major=0; major<(new_counts.size()/cat_stride); ++major) {
    for (int minor=0; minor<cat_stride; ++minor) {
      const int p = major*cat_stride+minor;
      unsigned& count = new_counts[p];
      for (int n=0; n<normal_count; ++n) {
        QList<int>& cell = horizontal ? d->cell(n, p) : d->cell(p, n);
        for (int i = 0; i<ncats; ++i) {
          const int old_parallel_index = major*source_stride+minor+i*cat_stride;
          QList<int> oldcell = oldcells.at(horizontal ? (old_parallel_index)*normal_count+n : n*old_counts.size()+old_parallel_index);
          cell.append(oldcell);
          count += oldcell.size();
          Q_FOREACH(int element, oldcell) {
            d->reverse_index.insert(element, horizontal ? cell_t(n,p) : cell_t(p, n));
          }
        }
      }
    }
  }
  emit reset();
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
}

int datacube_t::section_for_element(int element, Qt::Orientation orientation) const {
  const int section = d->compute_section_for_index(orientation, element);
  return orientation == Qt::Horizontal ? d->bucket_to_column(section) : d->bucket_to_row(section);
}

int datacube_t::section_for_element_internal(int element, Qt::Orientation orientation) const {
  if (orientation == Qt::Horizontal) {
    return d->bucket_to_column(d->reverse_index.value(element).column());
  } else  {
    return d->bucket_to_row(d->reverse_index.value(element).row());
  }

}

std::tr1::shared_ptr< abstract_filter_t > datacube_t::global_filter() const {
  return d->global_filter;
}

}

const QAbstractItemModel* qdatacube::datacube_t::underlying_model() const {
  return d->model;
}

void qdatacube::datacube_t::dump(bool cells, bool rowcounts, bool col_counts) const {
  if (col_counts) {
    qDebug() << "col_counts: " << d->col_counts;
  }
  if (rowcounts) {
    qDebug() << "row_counts: " << d->row_counts;
  }
  if (cells) {
    qDebug() << "Check: " << d->row_counts.size() << " * " << d->col_counts.size() << "=" << d->cells.size();
  }
  for (int r=0; r<d->row_counts.size(); ++r) {
    QList<int> row;
    for (int c=0; c<d->col_counts.size(); ++c) {
      row << d->cells.at(r+d->row_counts.size()*c).size();
    }
    qDebug() << row;
  }

}
void qdatacube::datacube_t::slot_filter_category_added(int index) {
  if (abstract_filter_t* filter = qobject_cast<abstract_filter_t*>(sender())) {
    int headerno = 0;
    Q_FOREACH(std::tr1::shared_ptr<abstract_filter_t> f, d->row_filters) {
      if (f.get() == filter) {
        filter_category_added(f, headerno,index, Qt::Vertical);
      }
      ++headerno;
    }
    headerno = 0;
    Q_FOREACH(std::tr1::shared_ptr<abstract_filter_t> f, d->col_filters) {
      if (f.get() == filter) {
        filter_category_added(f, headerno, index, Qt::Horizontal);
      }
      ++headerno;
    }
  }
}

void qdatacube::datacube_t::slot_filter_category_removed(int /*index*/) {
  Q_ASSERT(false); // TODO

}


void qdatacube::datacube_t::filter_category_added(std::tr1::shared_ptr< qdatacube::abstract_filter_t > filter, int headerno, int index, Qt::Orientation orientation)
{
  // NB! Since more filters might need to be adjusted, the col/row filters catogories() cannot be trusted downwards
  // and in the other direction. operator() cannot be trusted except for filter itself
  const secret_t::filters_t& parallel_filters = orientation == Qt::Horizontal ? d->col_filters : d->row_filters;
  QVector<unsigned>& new_parallel_counts = orientation == Qt::Horizontal ? d->col_counts : d->row_counts;
  const int normal_count = orientation == Qt::Horizontal ? d->row_counts.size() : d->col_counts.size();
  const QVector<unsigned> old_parallel_counts = new_parallel_counts;
  QVector<QList<int> > old_cells = d->cells;
  int nsuper_categories = 1;
  for (int h=0; h<headerno; ++h) {
    nsuper_categories *= parallel_filters[h]->categories(d->model).size();
  }
  const int new_ncats = filter->categories(d->model).size();
  const int old_ncats = new_ncats - 1;
  const int stride = old_parallel_counts.size()/old_ncats/nsuper_categories;
  Q_ASSERT(stride*nsuper_categories*old_ncats == old_parallel_counts.size());
  new_parallel_counts = QVector<unsigned>(new_ncats * nsuper_categories * stride);
  d->cells = QVector<QList<int> >(new_parallel_counts.size() * normal_count);
  for (int normal_index=0; normal_index<normal_count; ++normal_index) {
    for (int super_index=0; super_index<nsuper_categories; ++super_index) {
        for (int category_index=0; category_index<old_ncats; ++category_index) {
          for (int sub_index=0; sub_index<stride; ++sub_index) {
            const int old_p = super_index*stride*old_ncats + category_index*stride + sub_index;
            const int new_category_index = index<=category_index ? category_index+1 : category_index;
            const int p = super_index*stride*new_ncats + new_category_index*stride + sub_index;
            QList<int>& cell = orientation == Qt::Horizontal ? d->cell(normal_index, p) : d->cell(p, normal_index);
            cell = orientation == Qt::Horizontal ? old_cells.at(normal_index+ old_p*normal_count) : old_cells.at(normal_index*old_parallel_counts.size()+old_p);
            new_parallel_counts[p] = old_parallel_counts[old_p];
          }
        }
    }
  }
  emit reset(); // TODO: It is not impossible to emit the correct row/column changed instead
}

int qdatacube::datacube_t::bucket_for_column(int column) const
{
  return d->bucket_for_column(column);
}

int qdatacube::datacube_t::bucket_for_row(int row) const {
  return d->bucket_for_row(row);
}

QList<int> qdatacube::datacube_t::elements_in_bucket(int row, int column) const {
  return d->cell(row, column);

}

int qdatacube::datacube_t::number_of_buckets(Qt::Orientation orientation) const {
  return orientation == Qt::Vertical ? d->row_counts.size() : d->col_counts.size();
}

void qdatacube::datacube_t::add_selection_model(qdatacube::datacube_selection_t* selection) {
  d->selection_models << selection;
  connect(selection, SIGNAL(destroyed(QObject*)), SLOT(remove_selection_model(QObject*)));
}

int qdatacube::datacube_t::section_for_bucket_column(int bucket_column) const {
  return d->bucket_to_column(bucket_column);
}

int qdatacube::datacube_t::section_for_bucket_row(int bucket_row) const {
  return d->bucket_to_row(bucket_row);
}

void qdatacube::datacube_t::remove_selection_model(QObject* selection_model) {
  d->selection_models.removeAll(static_cast<datacube_selection_t*>(selection_model));
}

#include "datacube.moc"
