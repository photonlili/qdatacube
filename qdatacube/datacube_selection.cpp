#include "datacube_selection.h"
#include "datacube.h"
#include <QVector>
#include "datacube_view.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <QSet>
#include "cell.h"
#include <QAbstractProxyModel>

namespace qdatacube {

struct datacube_selection_t::secret_t {
  secret_t();
  datacube_t* datacube;
  QVector<int> cells;
  QSet<int> selected_elements;
  QItemSelectionModel* synchronized_selection_model;
  int nrows;
  int ncolumns;

  int& cell(int row, int column);

  void dump();

  void select_on_synchronized(QList<int> elements);
  void deselect_on_synchronized(QList<int> elements);
  void clear_synchronized();
  QItemSelection map_to_synchronized(QList<int> elements);
  QList<int> elements_from_selection(QItemSelection selection);
};

QList< int > datacube_selection_t::secret_t::elements_from_selection(QItemSelection selection) {
  QList<int> rv;
  if (synchronized_selection_model) {
    // Create list of rows (that is, indexes of the first column)
    QList<QModelIndex> indexes;
    const QAbstractItemModel* model = synchronized_selection_model->model();
    Q_FOREACH(QItemSelectionRange range, selection) {
      Q_ASSERT(synchronized_selection_model->model() == range.model());
      for (int row=range.top(); row<=range.bottom(); ++row) {
        indexes << model->index(row, 0);
      }
    }

    // Map that list all the way back to the source model
    while(const QAbstractProxyModel* proxy = qobject_cast<const QAbstractProxyModel*>(model)) {
      QList<QModelIndex> mapped_indexes;
      Q_FOREACH(const QModelIndex& unmapped_index, indexes) {
        mapped_indexes << proxy->mapToSource(unmapped_index);
      }
      indexes = mapped_indexes;
      model = proxy->sourceModel();
    }
    // Check that we did indeed get back to the underlying model
    if (model == datacube->underlying_model()) {
      // Finally, convert to elements = rows in underlying model
      Q_FOREACH(const QModelIndex& index, indexes) {
        rv << index.row();
      }
    } else {
      qWarning("Unable to map selection to underlying model");
    }
  }
  return rv;

}

QItemSelection datacube_selection_t::secret_t::map_to_synchronized(QList< int > elements) {
  QItemSelection selection;
  if (synchronized_selection_model) {
    // Get the reversed list of proxies to source model
    const QAbstractItemModel* model = synchronized_selection_model->model();
    const QAbstractItemModel* underlying_model = datacube->underlying_model();
    QList<const QAbstractProxyModel*> proxies;
    while (model != underlying_model) {
      if (const QAbstractProxyModel* proxy = qobject_cast<const QAbstractProxyModel*>(model)) {
        proxies << proxy;
        model = proxy->sourceModel();
      } else {
        qWarning("Unable to select on synchronized model");
        return QItemSelection();
      }
    }
    std::reverse(proxies.begin(), proxies.end());

    // Create selection on source model
    Q_FOREACH(int element, elements) {
      selection << QItemSelectionRange(model->index(element,0),model->index(element,0));
    }

    // Map from source to whatever proxy model the synchronized model
    Q_FOREACH(const QAbstractProxyModel* proxy, proxies) {
      QItemSelection proxyselection;
      Q_FOREACH(QItemSelectionRange range, selection) {
        QModelIndex mapped = proxy->mapFromSource(range.topLeft());
        proxyselection << QItemSelectionRange(mapped, mapped);
      }
      selection = proxyselection;
    }
  }
  return selection;
}

void datacube_selection_t::secret_t::select_on_synchronized(QList<int> elements) {
  if (synchronized_selection_model) {
    // Select on sync. model
    synchronized_selection_model->select(map_to_synchronized(elements), QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }

}

void datacube_selection_t::secret_t::deselect_on_synchronized(QList< int > elements) {
  if (synchronized_selection_model) {
    // Select on sync. model
    synchronized_selection_model->select(map_to_synchronized(elements), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  }

}
void datacube_selection_t::secret_t::clear_synchronized() {
  if (synchronized_selection_model) {
    synchronized_selection_model->clearSelection();
  }
}

datacube_selection_t::secret_t::secret_t() :
    datacube(0L),
    synchronized_selection_model(0L),
    nrows(0),
    ncolumns(0)
{

}

void datacube_selection_t::secret_t::dump() {
  for (int r = 0; r < nrows; ++r) {
    for (int c = 0; c < ncolumns; ++c) {
      std::cout << std::setw(3) << cell(r, c) << " ";
    }
    std::cout << "\n";
  }
  std::cout << std::endl;
}

int& datacube_selection_t::secret_t::cell(int row, int column) {
  return cells[row+column*nrows];

}

void datacube_selection_t::add_elements(QList< int > elements) {
  QList<int> actually_selected_elements;
  cell_t cell;
  Q_FOREACH(int element, elements) {
    if (!d->selected_elements.contains(element)) {
      d->datacube->bucket_for_element(element, cell);
      d->selected_elements << element;
      actually_selected_elements << element;
      if (!cell.invalid()) {
        int newvalue = ++d->cell(cell.row(), cell.column());
        if (newvalue == 1 || newvalue == d->datacube->elements_in_bucket(cell.row(), cell.column()).size()) {
          const int row_section = d->datacube->section_for_bucket_row(cell.row());
          const int column_section = d->datacube->section_for_bucket_column(cell.column());
          emit selection_status_changed(row_section,column_section);
        }
      }
    }
  }
  d->select_on_synchronized(actually_selected_elements);
}

void datacube_selection_t::remove_elements(QList< int > elements) {
  QList<int> actually_deselected_elements;
  cell_t cell;
  Q_FOREACH(int element, elements) {
    if (d->selected_elements.remove(element)) {
      d->datacube->bucket_for_element(element, cell);
      actually_deselected_elements << element;
      if (!cell.invalid()) {
        int newvalue = --d->cell(cell.row(), cell.column());
        Q_ASSERT(newvalue>=0);
        if (newvalue == 0 || newvalue == d->datacube->elements_in_bucket(cell.row(), cell.column()).size()-1) {
          const int row_section = d->datacube->section_for_bucket_row(cell.row());
          const int column_section = d->datacube->section_for_bucket_column(cell.column());
          emit selection_status_changed(row_section,column_section);
        }
      }
    }
  }
  d->deselect_on_synchronized(actually_deselected_elements);

}


datacube_selection_t::datacube_selection_t (qdatacube::datacube_t* datacube, qdatacube::datacube_view_t* view) :
    QObject(view),
    d(new secret_t) {
  d->datacube = datacube;
  datacube->add_selection_model(this);
  connect(d->datacube, SIGNAL(reset()), SLOT(reset()));
  reset();
}

void datacube_selection_t::reset() {
  d->nrows = d->datacube->number_of_buckets(Qt::Vertical);
  d->ncolumns = d->datacube->number_of_buckets(Qt::Horizontal);
  d->cells.resize(d->ncolumns*d->nrows);
  std::fill(d->cells.begin(), d->cells.end(), 0);
  QList<int> old_selected_elements = d->selected_elements.toList();
  d->selected_elements.clear();
  add_elements(old_selected_elements);

}

void datacube_selection_t::add_cell(int row, int column) {
  int bucket_row = d->datacube->bucket_for_row(row);
  int bucket_column = d->datacube->bucket_for_column(column);
  int& cell = d->cell(bucket_row, bucket_column);
  QList<int> raw_elements = d->datacube->elements_in_bucket(bucket_row, bucket_column);
  QList<int> elements;
  Q_FOREACH(int raw_element, raw_elements) {
    if (!d->selected_elements.contains(raw_element)) {
      elements << raw_element;
    }
  }
  if (!elements.isEmpty()) {
    cell += elements.size();
    Q_FOREACH(int element, elements) {
      d->selected_elements << element;
    }
    emit selection_status_changed(row, column);
    d->select_on_synchronized(elements);
  }
}

void datacube_selection_t::datacube_adds_element_to_bucket(int row, int column, int element) {
  if (d->selected_elements.contains(element)) {
    int& cell = d->cell(row, column);
    ++cell;
    Q_ASSERT(cell <= d->datacube->elements_in_bucket(row, column).size());
  }
}

void datacube_selection_t::datacube_removes_element_from_bucket(int row, int column, int element) {
  if (d->selected_elements.contains(element)) {
    int& cell = d->cell(row, column);
    --cell;
    Q_ASSERT(cell >= 0);
  }
}

datacube_selection_t::selection_status_t datacube_selection_t::selection_status(int row, int column) const {
  const int bucket_row = d->datacube->bucket_for_row(row);
  const int bucket_column = d->datacube->bucket_for_column(column);
  const int selected_count = d->cell(bucket_row, bucket_column);
  if (selected_count > 0) {
    const int count = d->datacube->elements_in_bucket(bucket_row, bucket_column).size();
    if (selected_count == count) {
      return SELECTED;
    } else {
      return PARTIALLY_SELECTED;
    }
  } else {
    return UNSELECTED;
  }

}

void datacube_selection_t::clear() {
  d->selected_elements.clear();
  std::fill(d->cells.begin(), d->cells.end(), 0);
  d->clear_synchronized();
}

void datacube_selection_t::update_selection(QItemSelection select, QItemSelection deselect) {
  add_elements(d->elements_from_selection(select));
  remove_elements(d->elements_from_selection(deselect));
}

datacube_selection_t::~datacube_selection_t() {
  // declared to have secret_t in scope
}

void datacube_selection_t::synchronize_with(QItemSelectionModel* synchronized_selection_model) {
  if (d->synchronized_selection_model) {
    d->synchronized_selection_model->disconnect(SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(update_selection(QItemSelection,QItemSelection)));
    d->synchronized_selection_model = 0L;
  }
  if (synchronized_selection_model) {
    d->synchronized_selection_model = synchronized_selection_model;
    connect(synchronized_selection_model,
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          SLOT(update_selection(QItemSelection,QItemSelection)));
    update_selection(synchronized_selection_model->selection(), QItemSelection());
  }

}


} // end of namespace

#include "datacube_selection.moc"
