/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

// Enable this to get a lot of consistency checks
//#define ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS 1

#include "datacube.h"
#include "abstract_aggregator.h"
#include "abstract_filter.h"

#include <QVector>
#include <algorithm>

#include <QAbstractItemModel>
#include "cell.h"
#include "datacube_selection.h"
using std::tr1::shared_ptr;

namespace qdatacube {

struct CellPoint {
    long row;
    long column;
    CellPoint(int row, int column) : row(row), column(column) {}
};

class datacube_t::secret_t {
  public:
    secret_t(const QAbstractItemModel* model,
             shared_ptr<AbstractAggregator> row_aggregator,
             shared_ptr<AbstractAggregator> column_aggregator);
    secret_t(const QAbstractItemModel* model);
    int compute_row_section_for_index(int index) {
      return compute_section_for_index(Qt::Vertical, index);
    }
    int compute_column_section_for_index(int index) {
      return compute_section_for_index(Qt::Horizontal, index);
    }
    int compute_section_for_index(Qt::Orientation orientation, int index);
    const QList<int>& cell(long int bucket_row, long int bucket_column) const;
    int hasCell(long int bucket_row, long int bucket_column) const;
    void setCell(long int bucket_row, long int bucket_column, QList< int > cell_content);
    void setCell(CellPoint point, QList<int> cell_content);
    void cellAppend(long int bucket_row, long int bucket_column, int to_add);
    void cellAppend(CellPoint point, QList<int> listadd);
    int bucket_to_row(int bucket_row) const;
    int bucket_to_column(int bucket_column) const;
    int bucket_for_row(int row);
    int bucket_for_column(int column);
    /**
     * Renumber cells from start by adding adjustment
     */
    void renumber_cells(int start, int adjustment);
    bool cellRemoveOne(long int row, long int column, int index);

    const QAbstractItemModel* model;
    typedef QVector<shared_ptr<AbstractAggregator> >aggregators_t;
    aggregators_t row_aggregators;
    aggregators_t col_aggregators;
    QVector<unsigned> row_counts;
    QVector<unsigned> col_counts;
    typedef QHash<long, QList<int> > cells_t;
    global_filters_t global_filters;
    typedef QHash<int, Cell> reverse_index_t;
    reverse_index_t reverse_index;
    QList<datacube_selection_t*> selection_models;
    const QList<int> empty_list;
    cells_t cells;
};


int datacube_t::secret_t::compute_section_for_index(Qt::Orientation orientation, int index) {
  qdatacube::datacube_t::secret_t::aggregators_t& aggregators = orientation == Qt::Horizontal ? col_aggregators : row_aggregators;
  int stride = 1;
  int rv = 0;
  for (int aggregator_index = aggregators.size()-1; aggregator_index>=0; --aggregator_index) {
    const shared_ptr<AbstractAggregator>& aggregator = aggregators.at(aggregator_index);
    rv += stride * (*aggregator)(index);
    stride *= aggregator->categoryCount();
  }
  Q_ASSERT(rv >=0);
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

const QList< int >& datacube_t::secret_t::cell(long bucket_row, long bucket_column) const {
  const long i = bucket_row + bucket_column*row_counts.size();
  cells_t::const_iterator it = cells.constFind(i);
  if(it == cells.constEnd()) {
    return empty_list;
  }
  return it.value();
}

void datacube_t::secret_t::cellAppend(CellPoint point, QList< int > listadd) {
    const long i = point.row + point.column*row_counts.size();
    cells[i].append(listadd);
}

void datacube_t::secret_t::cellAppend(long int bucket_row, long bucket_column, int to_add) {
    const long i = bucket_row + bucket_column*row_counts.size();
    cells[i].append(to_add);
}

bool datacube_t::secret_t::cellRemoveOne(long row, long column, int index) {
    const long i = row + column*row_counts.size();
    cells_t::iterator it = cells.find(i);
    if(it == cells.end()) {
        return false;
    }
    bool success = it.value().removeOne(index);
    if(it.value().isEmpty()) {
        cells.remove(i);
    }
    return success;
}

int datacube_t::secret_t::hasCell(long int bucket_row, long bucket_column) const {
    const long i = bucket_row + bucket_column*row_counts.size();
    cells_t::const_iterator it = cells.constFind(i);
    return it != cells.constEnd();
}

void datacube_t::secret_t::setCell(long int bucket_row, long bucket_column, QList< int > cell_content) {
    setCell(CellPoint(bucket_row, bucket_column), cell_content);
}

void datacube_t::secret_t::setCell(CellPoint point, QList< int > cell_content) {
    const long i = point.row + point.column*row_counts.size();
    cells_t::iterator it = cells.find(i);
    if(it == cells.end()) {
        cells.insert(i,cell_content);
    } else {
        if(cell_content.isEmpty()) {
            cells.erase(it);
        } else {
            *it = cell_content;
        }
    }
}



int datacube_t::secret_t::bucket_to_column(int bucket_column) const {
  int rv = 0;
  for (int i=0; i<bucket_column; ++i) {
    if (col_counts[i]>0) {
      ++rv;
    }
  }
  return rv;

}

int datacube_t::secret_t::bucket_to_row(int bucket_row) const {
  int rv = 0;
  for (int i=0; i<bucket_row; ++i) {
    if (row_counts[i]>0) {
      ++rv;
    }
  }
  return rv;

}
datacube_t::secret_t::secret_t(const QAbstractItemModel* model) :
                               model(model),
                               global_filters()
{
  col_counts = QVector<unsigned>(1);
  row_counts = QVector<unsigned>(1);
}

datacube_t::secret_t::secret_t(const QAbstractItemModel* model,
                               shared_ptr<AbstractAggregator> row_aggregator,
                               shared_ptr<AbstractAggregator> column_aggregator) :
    model(model),
    global_filters()
{
  col_aggregators << column_aggregator;
  row_aggregators << row_aggregator;
  col_counts = QVector<unsigned>(column_aggregator->categoryCount());
  row_counts = QVector<unsigned>(row_aggregator->categoryCount());
}

datacube_t::datacube_t(const QAbstractItemModel* model,
                       shared_ptr<AbstractAggregator> row_aggregator,
                       shared_ptr<AbstractAggregator> column_aggregator,
                       QObject* parent):
    QObject(parent),
    d(new secret_t(model, row_aggregator, column_aggregator))
{
  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(update_data(QModelIndex,QModelIndex)));
  connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(remove_data(QModelIndex,int,int)));
  connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(insert_data(QModelIndex,int,int)));
  connect(column_aggregator.get(), SIGNAL(categoryAdded(int)), SLOT(slot_aggregator_category_added(int)));
  connect(row_aggregator.get(), SIGNAL(categoryAdded(int)), SLOT(slot_aggregator_category_added(int)));
  connect(column_aggregator.get(), SIGNAL(categoryRemoved(int)), SLOT(slot_aggregator_category_removed(int)));
  connect(row_aggregator.get(), SIGNAL(categoryRemoved(int)), SLOT(slot_aggregator_category_removed(int)));;
  for (int element = 0, nelements = model->rowCount(); element < nelements; ++element) {
    add(element);
  }
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif

}

datacube_t::datacube_t(const QAbstractItemModel* model, QObject* parent)
  : QObject(parent),
    d(new secret_t(model))
{
  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(update_data(QModelIndex,QModelIndex)));
  connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(remove_data(QModelIndex,int,int)));
  connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(insert_data(QModelIndex,int,int)));
  for (int element = 0, nelements = model->rowCount(); element < nelements; ++element) {
    add(element);
  }
  #ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
}


void datacube_t::add_global_filter(std::tr1::shared_ptr< qdatacube::AbstractFilter > filter) {
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
  if (!filter) {
    return;
  }
  for (int row = 0, nrows = d->model->rowCount(); row<nrows; ++row) {
    const bool was_included = d->reverse_index.contains(row);
    const bool included = (*filter)(row);
    if (was_included && !included) {
      remove(row);
    }
  }
  d->global_filters << filter;
  emit global_filter_changed();
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
}

void datacube_t::remove_global_filter(std::tr1::shared_ptr< qdatacube::AbstractFilter > filter)
{
  remove_global_filter(filter.get());
  emit global_filter_changed();
}


bool datacube_t::remove_global_filter(qdatacube::AbstractFilter* filter) {
  for (global_filters_t::iterator it = d->global_filters.begin(), iend = d->global_filters.end(); it != iend; ++it) {
    if (it->get() == filter) {
      global_filters_t::value_type removed_filter = *it;
      d->global_filters.erase(it);
      for (int row = 0, nrows = d->model->rowCount(); row<nrows; ++row) {
        if (filtered_in(row)) {
          const bool excluded = !(*filter)(row);
          if (excluded) {
            add(row);
          }
        }
      }
      emit global_filter_changed();
      return true;
    }
  }
  return false;
}

void datacube_t::check() const {
    qDebug() << "running check" << d->col_counts.size() << d->row_counts.size() << d->cells.size();
  int total_count = 0;
  const int nelements = d->model->rowCount();
  for (int i=0; i<nelements;  ++i) {
    bool included = true;
    for (int filter_index=0, last_filter_index = d->global_filters.size()-1; filter_index<=last_filter_index; ++filter_index) {
      global_filters_t::const_reference filter = d->global_filters.at(filter_index);
      if (!(*filter)(i)) {
        included = false;
        break;
      }
    }
    if (included) {
      ++total_count;
    }
  }
  int failcols = 0;
  int failrows = 0;
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
      failcols++;
      Q_ASSERT(col_count == d->col_counts[c]);
    }
    count += col_count;
  }
  Q_ASSERT_X(count == total_count, __func__, QString("%1 == %2").arg(count).arg(total_count).toLocal8Bit().data());
  for (int i=0; i<d->row_counts.size(); ++i) {
        if(check_row_counts[i] != d->row_counts[i]) {
            failrows++;
            Q_ASSERT(check_row_counts[i] == d->row_counts[i]);
        }
  }
    qDebug() << "check done" << failcols << failrows;
}

void datacube_t::reset_global_filter() {
  if (d->global_filters.empty()) {
    return;
  }
  d->global_filters.clear();
  for (int row = 0, nrows = d->model->rowCount(); row<nrows; ++row) {
    const bool was_included = d->reverse_index.contains(row);
    if (!was_included) {
      add(row);
    }
  }
  emit global_filter_changed();

}

int datacube_t::header_count(Qt::Orientation orientation) const {
  return orientation == Qt::Horizontal ? d->col_aggregators.size() : d->row_aggregators.size();
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

QList< datacube_t::HeaderDescription > datacube_t::headers(Qt::Orientation orientation, int index) const {
  QList< HeaderDescription > rv;
  QVector<shared_ptr<AbstractAggregator> >& aggregators = (orientation == Qt::Horizontal) ? d->col_aggregators : d->row_aggregators;
  const QVector<unsigned>& counts = (orientation == Qt::Horizontal) ? d->col_counts : d->row_counts;
  shared_ptr<AbstractAggregator> aggregator = aggregators.at(index);
  const int ncats = aggregator->categoryCount();
  int stride = 1;
  for (int i=index+1; i<aggregators.size(); ++i) {
    stride *= aggregators.at(i)->categoryCount();
  }
  for (int c=0; c<counts.size(); c+=stride) {
    int count = 0;
    for (int i=0;i<stride; ++i) {
      if (counts.at(c+i)>0) {
        ++count;
      }
    }
    if (count > 0 ) {
      rv << HeaderDescription((c/stride)%ncats, count);
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
  d->cellAppend(row_section, column_section,index);
  d->reverse_index.insert(index, Cell(row_section, column_section));

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
  Cell cell = d->reverse_index.value(index);
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
  Q_ASSERT(d->hasCell(cell.row(),cell.column()));
  const bool check = d->cellRemoveOne(cell.row(), cell.column(),index);
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
    const bool filtered_out = !filtered_in(element);
    int new_row_section = d->compute_section_for_index(Qt::Vertical, element);
    int new_column_section = d->compute_section_for_index(Qt::Horizontal, element);
    Cell old_cell = d->reverse_index.value(element);
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
  Q_UNUSED(parent);
  Q_FOREACH(datacube_selection_t* selection, d->selection_models) {
    selection->datacube_inserts_elements(start, end);
  }
  d->renumber_cells(start, end-start+1);
  for (int row = start; row <=end; ++row) {
    if(filtered_in(row)) {
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
  Q_UNUSED(parent);
  for (int row = end; row>=start; --row) {
    remove(row);
  }
  Q_FOREACH(datacube_selection_t* selection, d->selection_models) {
    selection->datacube_deletes_elements(start, end);
  }
  // Now, all the remaining elements have to be renumbered
  d->renumber_cells(end+1, start-end-1);

}

void datacube_t::secret_t::renumber_cells(int start, int adjustment) {
  reverse_index_t new_index;
  for (cells_t::iterator it = cells.begin(), iend = cells.end(); it != iend; ++it) {
    for (QList<int>::iterator jit = it->begin(), jend = it->end(); jit != jend; ++jit) {
      Cell cell = reverse_index.value(*jit);
      if (*jit >= start) {
        *jit += adjustment;
      }
      new_index.insert(*jit, cell);
    }
  }
  reverse_index = new_index;

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

void datacube_t::split(Qt::Orientation orientation, int headerno, std::tr1::shared_ptr< AbstractAggregator > aggregator) {
  emit about_to_be_reset();
  if (orientation == Qt::Vertical) {
    split_row(headerno, aggregator);
  } else {
    split_column(headerno, aggregator);
  }
  connect(aggregator.get(), SIGNAL(categoryAdded(int)), SLOT(slot_aggregator_category_added(int)));
  connect(aggregator.get(), SIGNAL(categoryRemoved(int)), SLOT(slot_aggregator_category_removed(int)));;
  emit reset();
}

void datacube_t::split_row(int headerno, std::tr1::shared_ptr< AbstractAggregator > aggregator)
{
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
  const int ncats = aggregator->categoryCount();
  const int source_row_count = d->row_counts.size();
  const long target_row_countl = long(source_row_count) * long(ncats);
  if(INT_MAX/10 < target_row_countl) {
    qWarning("We are overflowing! Avoiding it by not splitting row.");
    return;
  }
  const int target_row_count = target_row_countl;
  emit about_to_be_reset();
  secret_t::cells_t oldcells = d->cells;
  d->cells = secret_t::cells_t();
  int cat_stride = 1;
  for (int i=headerno; i<d->row_aggregators.size(); ++i) {
    cat_stride *= d->row_aggregators.at(i)->categoryCount();
  }
  int target_stride = cat_stride*ncats;
  QVector<unsigned> old_row_counts = d->row_counts;
  d->row_counts = QVector<unsigned>(old_row_counts.size()*ncats);
  d->reverse_index = secret_t::reverse_index_t();
  // Sort out elements in new categories. Note that the old d->col_counts are unchanged
  for(secret_t::cells_t::const_iterator it = oldcells.constBegin(), end = oldcells.constEnd(); it!= end; ++it) {
        const long index = it.key();
        const long c = index / source_row_count;
        const long r = index % source_row_count;
        const long major = r / cat_stride;
        const long minor = r % cat_stride;
        Q_FOREACH(int element,it.value()) {
            const long target_row = major*target_stride + minor + (*aggregator).operator()(element) * cat_stride;
            const long target_index = target_row + long(c)*target_row_count;
            d->cells[target_index] << element;
            ++d->row_counts[target_row];
            d->reverse_index.insert(element, Cell(target_row, c));
        }
  }
  d->row_aggregators.insert(headerno, aggregator);
  emit reset();
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif

}

void datacube_t::split_column(int headerno, std::tr1::shared_ptr< AbstractAggregator > aggregator) {
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
  const int ncats = aggregator->categoryCount();
  const int old_column_count = d->col_counts.size();
  const long new_column_countl = long(old_column_count) * long(ncats);
  if(INT_MAX/10 < new_column_countl) {
    qWarning("We are overflowing! Avoiding it by not splitting column.");
    return;
  }
  emit about_to_be_reset();
  secret_t::cells_t oldcells = d->cells;
  d->cells = secret_t::cells_t();
  const int row_count = d->row_counts.size();
  int cat_stride = 1;
  for (int i=headerno; i<d->col_aggregators.size(); ++i) {
    cat_stride *= d->col_aggregators.at(i)->categoryCount();
  }
  int target_stride = cat_stride*ncats;
  QVector<unsigned> old_column_counts = d->col_counts;
  d->col_counts = QVector<unsigned>(int(new_column_countl));
  d->reverse_index = secret_t::reverse_index_t();
  // Sort out elements in new categories. Note that the old d->row_counts are unchanged
 
  for(secret_t::cells_t::const_iterator it = oldcells.constBegin(), end = oldcells.constEnd(); it!= end; ++it) {
        const long index = it.key();
        const long r = index % row_count;
        const long c = index/row_count;
        const long major = c/cat_stride;
        const long minor = c%cat_stride;
        Q_FOREACH(int element,it.value()) {
            const long target_column = major*target_stride + minor + (*aggregator).operator()(element) * cat_stride;
            const long target_index = r + long(target_column)*d->row_counts.size();
            d->cells[target_index] << element;
            d->reverse_index.insert(element, Cell(r, target_column));
            ++d->col_counts[target_column];
        }
  }
  d->col_aggregators.insert(headerno, aggregator);
  emit reset();
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif

}

void datacube_t::collapse(Qt::Orientation orientation, int headerno) {
#ifdef ANGE_QDATACUBE_CHECK_PRE_POST_CONDITIONS
  check();
#endif
  emit about_to_be_reset();
  secret_t::cells_t oldcells = d->cells;
  const bool horizontal = (orientation == Qt::Horizontal);
  secret_t::aggregators_t& parallel_aggregators = horizontal ? d->col_aggregators : d->row_aggregators;
  shared_ptr<AbstractAggregator> aggregator = parallel_aggregators[headerno];
  disconnect(aggregator.get(), SIGNAL(categoryAdded(int)), this, SLOT(slot_aggregator_category_added(int)));
  disconnect(aggregator.get(), SIGNAL(categoryRemoved(int)), this, SLOT(slot_aggregator_category_removed(int)));;
  parallel_aggregators.remove(headerno);
  const int ncats = aggregator->categoryCount();
  d->cells = secret_t::cells_t();
  const int normal_count = horizontal ? d->row_counts.size() : d->col_counts.size();
  int cat_stride = 1;
  for (int i=headerno; i<parallel_aggregators.size(); ++i) {
    cat_stride *= parallel_aggregators.at(i)->categoryCount();
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
        QList<int> cell;
        for (int i = 0; i<ncats; ++i) {
          const int old_parallel_index = major*source_stride+minor+i*cat_stride;
          QList<int> oldcell = oldcells.value(horizontal ? (old_parallel_index)*normal_count+n : n*old_counts.size()+old_parallel_index);
          cell.append(oldcell);
          count += oldcell.size();
          Q_FOREACH(int element, oldcell) {
            d->reverse_index.insert(element, horizontal ? Cell(n,p) : Cell(p, n));
          }
        }
        d->cellAppend(horizontal ? CellPoint(n, p) : CellPoint(p, n),cell);
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

void datacube_t::bucket_for_element(int element, Cell& result) const {
  result = d->reverse_index.value(element);
}

QList< std::tr1::shared_ptr< AbstractFilter > > datacube_t::global_filters() const {
  return d->global_filters;
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
      row << d->cells.value(r+d->row_counts.size()*c).size();
    }
    qDebug() << row;
  }

}
void qdatacube::datacube_t::slot_aggregator_category_added(int index) {
  if (AbstractAggregator* aggregator = qobject_cast<AbstractAggregator*>(sender())) {
    int headerno = 0;
    Q_FOREACH(std::tr1::shared_ptr<AbstractAggregator> f, d->row_aggregators) {
      if (f.get() == aggregator) {
        aggregator_category_added(f, headerno,index, Qt::Vertical);
      }
      ++headerno;
    }
    headerno = 0;
    Q_FOREACH(std::tr1::shared_ptr<AbstractAggregator> f, d->col_aggregators) {
      if (f.get() == aggregator) {
        aggregator_category_added(f, headerno, index, Qt::Horizontal);
      }
      ++headerno;
    }
  }
}

void qdatacube::datacube_t::slot_aggregator_category_removed(int index) {
  if (AbstractAggregator* aggregator = qobject_cast<AbstractAggregator*>(sender())) {
    int headerno = 0;
    Q_FOREACH(std::tr1::shared_ptr<AbstractAggregator> f, d->row_aggregators) {
      if (f.get() == aggregator) {
        aggregator_category_removed(f, headerno,index, Qt::Vertical);
      }
      ++headerno;
    }
    headerno = 0;
    Q_FOREACH(std::tr1::shared_ptr<AbstractAggregator> f, d->col_aggregators) {
      if (f.get() == aggregator) {
        aggregator_category_removed(f, headerno, index, Qt::Horizontal);
      }
      ++headerno;
    }
  }

}


void qdatacube::datacube_t::aggregator_category_added(std::tr1::shared_ptr< qdatacube::AbstractAggregator > aggregator, int headerno, int index, Qt::Orientation orientation)
{
  const secret_t::aggregators_t& parallel_aggregators = orientation == Qt::Horizontal ? d->col_aggregators : d->row_aggregators;
  QVector<unsigned>& new_parallel_counts = orientation == Qt::Horizontal ? d->col_counts : d->row_counts;
  const int normal_count = orientation == Qt::Horizontal ? d->row_counts.size() : d->col_counts.size();
  const QVector<unsigned> old_parallel_counts = new_parallel_counts;
  secret_t::cells_t old_cells = d->cells;
  int nsuper_categories = 1;
  for (int h=0; h<headerno; ++h) {
    nsuper_categories *= qMax(parallel_aggregators[h]->categoryCount(),1);
  }
  const int new_ncats = aggregator->categoryCount();
  int n_new_parallel_counts = nsuper_categories * new_ncats;
  for (int h = headerno+1; h<parallel_aggregators.size(); ++h) {
    n_new_parallel_counts *= parallel_aggregators[h]->categoryCount();
  }
  new_parallel_counts = QVector<unsigned>(n_new_parallel_counts);
  d->cells = secret_t::cells_t();
  if (!d->reverse_index.empty()) { // If there is no elements in the recap, it is possible some of the aggregators have no categories.
    const int old_ncats = new_ncats - 1;
    const int stride = old_parallel_counts.size()/old_ncats/nsuper_categories;
    Q_ASSERT(stride*nsuper_categories*old_ncats == old_parallel_counts.size());
    d->reverse_index.clear();
    for (int normal_index=0; normal_index<normal_count; ++normal_index) {
      for (int super_index=0; super_index<nsuper_categories; ++super_index) {
        for (int category_index=0; category_index<old_ncats; ++category_index) {
          for (int sub_index=0; sub_index<stride; ++sub_index) {
            const int old_p = super_index*stride*old_ncats + category_index*stride + sub_index;
            const int new_category_index = index<=category_index ? category_index+1 : category_index;
            const int p = super_index*stride*new_ncats + new_category_index*stride + sub_index;
            d->setCell(orientation == Qt::Horizontal ? CellPoint(normal_index, p) : CellPoint(p, normal_index),
                orientation == Qt::Horizontal ? old_cells.value(normal_index+ old_p*normal_count) : old_cells.value(normal_index*old_parallel_counts.size()+old_p));
            new_parallel_counts[p] = old_parallel_counts[old_p];
            Q_FOREACH(int element, orientation == Qt::Horizontal ? d->cell(normal_index, p) : d->cell(p, normal_index) ) {
              d->reverse_index.insert(element, orientation == Qt::Horizontal ? Cell(normal_index,p) : Cell(p, normal_index));
            }
          }
        }
      }
    }
  }
  emit reset(); // TODO: It is not impossible to emit the correct row/column changed instead
}

void qdatacube::datacube_t::aggregator_category_removed(std::tr1::shared_ptr< qdatacube::AbstractAggregator > aggregator, int headerno, int index, Qt::Orientation orientation)
{
  const secret_t::aggregators_t& parallel_aggregators = orientation == Qt::Horizontal ? d->col_aggregators : d->row_aggregators;
  QVector<unsigned>& new_parallel_counts = orientation == Qt::Horizontal ? d->col_counts : d->row_counts;
  const int normal_count = orientation == Qt::Horizontal ? d->row_counts.size() : d->col_counts.size();
  const QVector<unsigned> old_parallel_counts = new_parallel_counts;
  secret_t::cells_t old_cells = d->cells;
  int nsuper_categories = 1;
  for (int h=0; h<headerno; ++h) {
    nsuper_categories *= qMax(parallel_aggregators[h]->categoryCount(),1);
  }
  const int new_ncats = aggregator->categoryCount();
  const int old_ncats = new_ncats + 1;
  const int stride = old_parallel_counts.size()/old_ncats/nsuper_categories;
  Q_ASSERT(stride*nsuper_categories*old_ncats == old_parallel_counts.size());
  new_parallel_counts = QVector<unsigned>(new_ncats * nsuper_categories * stride);
  d->cells = secret_t::cells_t();
  d->reverse_index.clear();
  for (int normal_index=0; normal_index<normal_count; ++normal_index) {
    for (int super_index=0; super_index<nsuper_categories; ++super_index) {
      for (int category_index=0; category_index<new_ncats; ++category_index) {
        for (int sub_index=0; sub_index<stride; ++sub_index) {
          const int old_category_index = index<=category_index ? category_index+1 : category_index;
          const int old_p = super_index*stride*old_ncats + old_category_index*stride + sub_index;
          const int p = super_index*stride*new_ncats + category_index*stride + sub_index;
          d->setCell(orientation == Qt::Horizontal ? CellPoint(normal_index, p) : CellPoint(p, normal_index),
             orientation == Qt::Horizontal ? old_cells.value(normal_index+ old_p*normal_count) : old_cells.value(normal_index*old_parallel_counts.size()+old_p));
          new_parallel_counts[p] = old_parallel_counts[old_p];
          Q_FOREACH(int element, orientation == Qt::Horizontal ? d->cell(normal_index, p) : d->cell(p, normal_index)) {
            d->reverse_index.insert(element, orientation == Qt::Horizontal ? Cell(normal_index,p) : Cell(p, normal_index));
          }
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

int qdatacube::datacube_t::category_index(Qt::Orientation orientation, int header_index, int section) const {
  const int bucket = (orientation == Qt::Vertical) ? d->bucket_for_row(section) : d->bucket_for_column(section);
  int sub_header_size = 1;
  const secret_t::aggregators_t& aggregators = (orientation == Qt::Vertical) ? d->row_aggregators : d->col_aggregators;
  for (int i=header_index+1; i<aggregators.size(); ++i) {
    sub_header_size *= aggregators[i]->categoryCount();
  }
  const int naggregator_categories = aggregators[header_index]->categoryCount();
  return bucket % (naggregator_categories*sub_header_size)/sub_header_size;
}

bool qdatacube::datacube_t::filtered_in(int element) const {
  Q_FOREACH(global_filters_t::value_type filter, d->global_filters) {
    if (!(*filter)(element)) {
      return false;
    }
  }
  return true;
}

qdatacube::datacube_t::aggregators_t qdatacube::datacube_t::column_aggregators() const
{
  return d->col_aggregators.toList();
}

qdatacube::datacube_t::aggregators_t qdatacube::datacube_t::row_aggregators() const
{
  return d->row_aggregators.toList();
}

int qdatacube::datacube_t::element_count(Qt::Orientation orientation, int headerno, int header_section) const
{
  QVector<shared_ptr<AbstractAggregator> >& aggregators = (orientation == Qt::Horizontal) ? d->col_aggregators : d->row_aggregators;
  const QVector<unsigned>& counts = (orientation == Qt::Horizontal) ? d->col_counts : d->row_counts;
  int count = 0;
  int stride = 1;
  for (int i=headerno+1; i<aggregators.size(); ++i) {
    stride *= aggregators.at(i)->categoryCount();
  }
  int offset = 0;
  for (int section_index = 0; section_index <= header_section; section_index += (count>0) ? 1 : 0) {
    Q_ASSERT_X(offset < counts.size(), "QDatacube", QString("Section %1 at header %2 orientation %3 too big for qdatacube").arg(header_section).arg(headerno).arg(orientation == Qt::Horizontal ? "Horizontal" : "Vertical").toLocal8Bit().data());
    count = 0;
    for (int i=0; i<stride; ++i) {
      count += counts.at(offset+i);
    }
    offset += stride;
  }
  return count;
}

QList<int> qdatacube::datacube_t::elements(Qt::Orientation orientation, int headerno, int header_section) const
{
  QVector<shared_ptr<AbstractAggregator> >& aggregators = (orientation == Qt::Horizontal) ? d->col_aggregators : d->row_aggregators;
  const QVector<unsigned>& counts = (orientation == Qt::Horizontal) ? d->col_counts : d->row_counts;
  int stride = 1;
  for (int i=headerno+1; i<aggregators.size(); ++i) {
    stride *= aggregators.at(i)->categoryCount();
  }
  // Skip forward to section
  int bucket = 0;
  int section = 0;
  for (int hs=0; bucket<counts.size() && hs < header_section; bucket+=stride) {
    int old_section = section;
    for (int i=0;i<stride; ++i) {
      if (counts.at(bucket+i)>0) {
        ++section;
      }
    }
    if (section>old_section) {
      ++hs;
    }
  }

  // Accumulate answer
  QList<int> rv;
  const int normal_count = (orientation == Qt::Horizontal) ? d->row_counts.size() : d->col_counts.size();
  for (; bucket<counts.size() && rv.isEmpty(); bucket+=stride) {
    for (int i=0;i<stride && (bucket+i)<counts.size(); ++i) {
      if (counts.at(bucket+i)>0) {
        for (int n=0; n<normal_count; ++n) {
          rv << ((orientation == Qt::Horizontal) ? d->cell(n,bucket+i) : d->cell(bucket+i,n));
        }
      }
    }
  }
  return rv;

}

int qdatacube::datacube_t::to_header_section(const Qt::Orientation orientation, const int headerno, const int section) const
{
//   qDebug() << __func__ << headerno << section << row_count() << column_count();
  QVector<shared_ptr<AbstractAggregator> >& aggregators = (orientation == Qt::Horizontal) ? d->col_aggregators : d->row_aggregators;
  const QVector<unsigned>& counts = (orientation == Qt::Horizontal) ? d->col_counts : d->row_counts;
  int stride = 1;
  for (int i=headerno+1; i<aggregators.size(); ++i) {
    stride *= aggregators.at(i)->categoryCount();
  }
  // Skip forward to section
  int bucket = 0;
  int s = 0;
  int header_section = 0;
  for (; bucket<counts.size(); bucket+=stride) {
    int old_section = s;
    for (int i=0;i<stride; ++i) {
//       qDebug() << __func__ << bucket << s << header_section << stride << counts.at(bucket+i) << counts.size();
      if (counts.at(bucket+i)>0) {
        if (s++==section) {
          return header_section;
        }
      }
    }
    if (s>old_section) {
      ++header_section;
    }
  }
  Q_ASSERT_X(bucket<counts.size(), "QDatacube", QString("Section %1 in datacube orientation %3 too big for qdatacube").arg(section).arg(headerno).arg(orientation == Qt::Horizontal ? "Horizontal" : "Vertical").toLocal8Bit().data());
  return header_section;
}

QPair< int, int > qdatacube::datacube_t::to_section(Qt::Orientation orientation, const int headerno, const int header_section) const
{
  QVector<shared_ptr<AbstractAggregator> >& aggregators = (orientation == Qt::Horizontal) ? d->col_aggregators : d->row_aggregators;
  const QVector<unsigned>& counts = (orientation == Qt::Horizontal) ? d->col_counts : d->row_counts;
  int stride = 1;
  for (int i=headerno+1; i<aggregators.size(); ++i) {
    stride *= aggregators.at(i)->categoryCount();
  }
  // Skip forward to section
  int bucket = 0;
  int section = 0;
  for (int hs=0; bucket<counts.size() && hs < header_section; bucket+=stride) {
    int old_section = section;
    for (int i=0;i<stride; ++i) {
      if (counts.at(bucket+i)>0) {
        ++section;
      }
    }
    if (section>old_section) {
      ++hs;
    }
  }

  // count number of (active) sections
  int count = 0;
  for (; bucket<counts.size() && count == 0; bucket+=stride) {
    for (int i=0;i<stride && (bucket+i)<counts.size(); ++i) {
      if (counts.at(bucket+i)>0) {
        ++count;
      }
    }
  }
  Q_ASSERT_X(count > 0, "QDatacube", QString("Section %1 in header %2 orientation %3 too big for qdatacube").arg(header_section).arg(headerno).arg(orientation == Qt::Horizontal ? "Horizontal" : "Vertical").toLocal8Bit().data());
  return QPair<int,int>(section,section+count-1);
}

int qdatacube::datacube_t::element_count() const
{
  return d->reverse_index.count();
}

QList< int > qdatacube::datacube_t::elements() const
{
  return d->reverse_index.keys();
}

#include "datacube.moc"
