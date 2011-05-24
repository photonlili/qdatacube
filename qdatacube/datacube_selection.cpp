#include "datacube_selection.h"
#include "datacube.h"
#include <QVector>
#include "datacube_view.h"

#include <iostream>
#include <iomanip>
#include <QSet>

namespace qdatacube {

struct datacube_selection_t::secret_t {
  datacube_t* datacube;
  QVector<int> cells;
  QSet<int> selected_elements;
  int nrows;
  int ncolumns;

  int& cell(int row, int column);

  void dump();
};

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

void datacube_selection_t::add_element(int element) {
  const int column = d->datacube->section_for_element_internal(element, Qt::Horizontal);
  const int row = d->datacube->section_for_element_internal(element, Qt::Vertical);
  ++d->cell(row, column);
}

void datacube_selection_t::add_elements(QList< int > elements) {
  Q_FOREACH(int element, elements) {
    const int column = d->datacube->section_for_element_internal(element, Qt::Horizontal);
    const int row = d->datacube->section_for_element_internal(element, Qt::Vertical);
    ++d->cell(row, column);
  }
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

}

void datacube_selection_t::add_cell(int row, int column) {
  int bucket_row = d->datacube->bucket_for_row(row);
  int bucket_column = d->datacube->bucket_for_column(column);
  int& cell = d->cell(bucket_row, bucket_column);
  QList<int> elements = d->datacube->elements_in_bucket(bucket_row, bucket_column);
  if (cell != elements.size()) {
    cell = elements.size();
    Q_FOREACH(int element, elements) {
      d->selected_elements << element;
    }
    emit selection_status_changed(row, column);
  }
}

void datacube_selection_t::datacube_adds_element_to_bucket(int row, int column, int element) {
  if (d->selected_elements.contains(element)) {
    int& cell = d->cell(row, column);
    ++cell;
    Q_ASSERT(cell <= d->datacube->elements_in_bucket(row, column).size());
  }
}

void datacube_selection_t::datacube_removes_element_to_bucket(int row, int column, int element) {
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
}


} // end of namespace

#include "datacube_selection.moc"
