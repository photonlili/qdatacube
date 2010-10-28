#include "datacube_selection_model.h"
#include "datacube.h"
#include "datacube_model.h"
#include "abstract_filter.h"
#include <QAbstractProxyModel>
#include <QSize>

namespace qdatacube {

struct datacube_selection_model_t::secret_t {
  datacube_t* datacube;
  QList<int> select_count;
  QSize select_size;
  bool ignore_selections;

  secret_t() : ignore_selections(false) {}

  void clear();
  void cell_select(const QModelIndex& index);
  void cell_deselect(const QModelIndex& index);
  void row_select(const QAbstractItemModel* model, int row);
  void row_deselect(const QAbstractItemModel* model, int row);
  void column_select(const QAbstractItemModel* model, int column);
  void column_deselect(const QAbstractItemModel* model, int column);
};

void datacube_selection_model_t::secret_t::cell_deselect(const QModelIndex& index) {
  select_count[index.row()+index.column()*index.model()->rowCount()] = 0;
}

void datacube_selection_model_t::secret_t::cell_select(const QModelIndex& index) {
  int count = datacube->element_count(index.row(), index.column());
  select_count[index.row()+index.column()*index.model()->rowCount()] = count;

}
void datacube_selection_model_t::secret_t::row_deselect(const QAbstractItemModel* model, int row) {
  const int ncolumns = model->columnCount();
  for (int column=0; column<ncolumns; ++column) {
    cell_deselect(model->index(row, column));
  }
}

void datacube_selection_model_t::secret_t::row_select(const QAbstractItemModel* model, int row) {
  const int ncolumns = model->columnCount();
  for (int column=0; column<ncolumns; ++column) {
    cell_select(model->index(row, column));
  }

}

void datacube_selection_model_t::secret_t::column_deselect(const QAbstractItemModel* model, int column) {
  const int nrows = model->rowCount();
  for (int row=0; row<nrows; ++row) {
    cell_deselect(model->index(row, column));
  }
}

void datacube_selection_model_t::secret_t::column_select(const QAbstractItemModel* model, int column) {
  const int nrows = model->rowCount();
  for (int row=0; row<nrows; ++row) {
    cell_select(model->index(row, column));
  }
}

void datacube_selection_model_t::secret_t::clear() {
    for (int i=0; i<select_count.size(); ++i) {
      select_count[i] = 0;
    }
}

void datacube_selection_model_t::select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command) {
  if (d->ignore_selections) {
    QItemSelectionModel::select(index, command);
    return;
  }
  d->ignore_selections = true;
  if (command.testFlag(Clear)) {
    clear_implementation();
  }
  if (index.isValid()) {
    Q_ASSERT(!command.testFlag(Toggle)); // Not implemented
    if (command.testFlag(Columns)) {
      if (command.testFlag(Select)) {
        d->column_select(index.model(), index.column());
      } else if (command.testFlag(Deselect)) {
        d->column_deselect(index.model(), index.column());
      }
    } else if (command.testFlag(Rows)) {
      if (command.testFlag(Select)) {
        d->row_select(index.model(), index.row());
      } else if (command.testFlag(Deselect)) {
        d->row_deselect(index.model(), index.row());
      }
    } else {
      if (command.testFlag(Select)) {
        d->cell_select(index);
      } else if (command.testFlag(Deselect)) {
        d->cell_deselect(index);
      }
    }
  }
  QItemSelectionModel::select(index, command);
  d->ignore_selections = false;

}

void datacube_selection_model_t::select(const QItemSelection& selection, QItemSelectionModel::SelectionFlags command)
{
  if (d->ignore_selections) {
    QItemSelectionModel::select(selection, command);
    return;
  }
  Q_ASSERT(!command.testFlag(QItemSelectionModel::Toggle)); // Not implemented
  d->ignore_selections = true;
  if (command.testFlag(Clear)) {
    clear_implementation();
  }
  if (command.testFlag(Columns)) {
    if (command.testFlag(Select)) {
      Q_FOREACH(QItemSelectionRange range, selection) {
        for (int column = range.left(); column <= range.right(); ++column) {
          d->column_select(range.model(), column);
        }
      }
    } else if (command.testFlag(Deselect)) {
      Q_FOREACH(QItemSelectionRange range, selection) {
        for (int column = range.left(); column <= range.right(); ++column) {
          d->column_deselect(range.model(), column);
        }
      }
    }
  } else if (command.testFlag(Rows)) {
    if (command.testFlag(Select)) {
      Q_FOREACH(QItemSelectionRange range, selection) {
        for (int row = range.top(); row <= range.bottom(); ++row) {
          d->row_select(range.model(), row);
        }
      }
    } else if (command.testFlag(Deselect)) {
      Q_FOREACH(QItemSelectionRange range, selection) {
        for (int row = range.top(); row <= range.bottom(); ++row) {
          d->row_deselect(range.model(), row);
        }
      }
    }
  } else {
    if (command.testFlag(Select)) {
      Q_FOREACH(QItemSelectionRange range, selection) {
        for (int row = range.top(); row <= range.bottom(); ++row) {
          for (int column = range.left(); column <= range.right(); ++column) {
            d->cell_select(range.model()->index(row,column));
          }
        }
      }
    } else if (command.testFlag(Deselect)) {
      Q_FOREACH(QItemSelectionRange range, selection) {
        for (int row = range.top(); row <= range.bottom(); ++row) {
          for (int column = range.left(); column <= range.right(); ++column) {
            d->cell_deselect(range.model()->index(row, column));
          }
        }
      }
    }
  }
  QItemSelectionModel::select(selection, command);

  d->ignore_selections = false;
}

datacube_selection_model_t::datacube_selection_model_t(QAbstractItemModel* model):
    QItemSelectionModel(model),
    d(new datacube_selection_model_t::secret_t)
{
  if (datacube_model_t* m = qobject_cast<datacube_model_t*>(model)) {
    d->datacube = m->datacube();
  } else {
    d->datacube = 0L;
  }
  d->select_size = QSize(model->columnCount(), model->rowCount());
  for (int i=0, iend=d->select_size.width()*d->select_size.height(); i<iend; ++i) {
    d->select_count << 0;
  }
  connect(model, SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int, int)), SLOT(slot_insert_rows(const QModelIndex&, int, int)));
  connect(model, SIGNAL(columnsAboutToBeInserted(const QModelIndex&, int, int)), SLOT(slot_insert_columns(QModelIndex,int,int)));
}

void datacube_selection_model_t::select_elements(const QList< int >& elements) {
  if (d->ignore_selections) {
    return;
  }
  if (datacube_t* cube = d->datacube) {
    std::tr1::shared_ptr<abstract_filter_t> global_filter = cube->global_filter();
    const QAbstractItemModel* underlying_model = cube->underlying_model();
    QItemSelection selection;
    Q_FOREACH(int element, elements) {
      if (!global_filter.get() || (*global_filter)(underlying_model, element) == 0) {
        int row = cube->section_for_element(element, Qt::Vertical);
        int column = cube->section_for_element(element, Qt::Horizontal);
        int newcount = ++d->select_count[row+column*d->select_size.height()];
        if (newcount == d->datacube->element_count(row,column)) {
          QModelIndex index = model()->index(row,column);
          d->ignore_selections = true;
          select(index, QItemSelectionModel::Select);
          d->ignore_selections = false;
        }
        if (newcount == 1) {
          QModelIndex index = model()->index(row,column);
          selection.append(QItemSelectionRange(index));
        }
      }
    }
    if (!selection.isEmpty()) {
      emit partial_selection_changed(selection, QItemSelection());
    }
  }
}

void datacube_selection_model_t::deselect_elements(const QList< int >& elements) {
  if (d->ignore_selections) {
    return;
  }
  if (datacube_t* cube = d->datacube) {
    std::tr1::shared_ptr<abstract_filter_t> global_filter = cube->global_filter();
    const QAbstractItemModel* underlying_model = cube->underlying_model();
    QItemSelection deselection;
    Q_FOREACH(int element, elements) {
      if (!global_filter.get() || (*global_filter)(underlying_model, element) == 0) {
        int row = cube->section_for_element(element, Qt::Vertical);
        int column = cube->section_for_element(element, Qt::Horizontal);
        int oldcount = d->select_count[row+column*d->select_size.height()]--;
        if (oldcount == d->datacube->element_count(row,column)) {
          QModelIndex index = model()->index(row,column);
          d->ignore_selections = true;
          select(index, QItemSelectionModel::Deselect);
          d->ignore_selections = false;
        }
        if (oldcount==1) {
          QModelIndex index = model()->index(row,column);
          deselection.append(QItemSelectionRange(index));
        }
      }
    }
    if (!deselection.isEmpty()) {
      emit partial_selection_changed(QItemSelection(), deselection);
    }
  }
}

datacube_selection_model_t::~datacube_selection_model_t() {

}

void datacube_selection_model_t::slot_insert_rows ( const QModelIndex& , int begin, int end ) {
  const int n_rows = d->select_size.height();
  const int n_columns = d->select_size.width();
  for (int col=0; col<n_columns; ++col) {
    for (int row = begin; row<=end; ++row) {
      d->select_count.insert(col*n_rows+row, 0);
    }
  }
  d->select_size.rheight() += (end-begin+1);
  Q_ASSERT(d->select_size.width()*d->select_size.height() == d->select_count.size());
}

void datacube_selection_model_t::slot_insert_columns(const QModelIndex& , int begin, int end) {
  const int n_rows = d->select_size.height();
  for (int col=begin; col<=end; ++col) {
    for (int row = 0; row<n_rows; ++row) {
      d->select_count.insert(col*n_rows+row, 0);
    }
  }
  d->select_size.rwidth() += (end-begin+1);
  Q_ASSERT(d->select_size.width()*d->select_size.height() == d->select_count.size());
}

bool datacube_selection_model_t::partially_selected(const QModelIndex& index) const {
  const int count = d->select_count.at(index.column()*d->select_size.height()+index.row());
  #if 0
  return (count > 0 && count < d->datacube->element_count(index.row(), index.column()));
  #else
  return count>0;
  #endif
}

void datacube_selection_model_t::clear_implementation() {
  QItemSelection deselection;
  for (int column=0; column<d->select_size.width(); ++column) {
    for (int row=0; row<d->select_size.height(); ++row) {
      int count = d->select_count.at(column*d->select_size.height()+row);
      if (count>0) {
        QModelIndex index = model()->index(row,column);
        if (!isSelected(index)) {
          // Partially selected
          deselection.append(QItemSelectionRange(index));
        }
      }
    }
  }
  emit partial_selection_changed(QItemSelection(), deselection);
  d->clear();
}

} // end of namespace

#include "datacube_selection_model.moc"
