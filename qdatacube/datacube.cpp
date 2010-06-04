/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube.h"
#include "datacube_colrow.h"

#include <QAbstractItemModel>

namespace qdatacube {

datacube_t::datacube_t(const QAbstractItemModel* model,
                 abstract_filter_t* row_filter,
                 abstract_filter_t* column_filter,
                 const QList<int>& active,
                 QObject* parent):
    QObject(parent),
    m_columns(new datacube_colrow_t(model, column_filter, active)),
    m_rows(new datacube_colrow_t(model, row_filter, active)) {

}

int datacube_t::headerCount(Qt::Orientation orientation) const {
  return orientation == Qt::Horizontal ? m_columns->depth() : m_rows->depth();
}

int datacube_t::cellCount(int row, int column) const {
  return cellrows(row,column).size();
}

int datacube_t::columnCount() const {
  return m_columns->size();
}

int datacube_t::rowCount() const {
  return m_rows->size();
}

QList< int > datacube_t::cellrows(int row, int column) const {
  // Return the intersection of the row's and the column's container indexes
  QList<int> rowindex = m_rows->container_indexes(row);
  QList<int> colindex = m_columns->container_indexes(column);
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
    return m_columns->active_headers(index);
  } else {
    return m_rows->active_headers(index);
  }
}

datacube_t::~datacube_t() {
  // Need to declare here so datacube_colrow_t's destructor is visible
}

int datacube_t::depth(Qt::Orientation orientation) {
  return orientation == Qt::Horizontal ? m_columns->depth() : m_rows->depth();
}

void datacube_t::restrict(QList< int > set) {
  m_columns->restrict(set);
  m_rows->restrict(set);
  emit changed();
}

void datacube_t::remove(int container_index) {
  int row_index = m_rows->bucket_for_container(container_index);
  if (row_index == -1) {
    // Our qdatacube does not cover that container. Just ignore it.
    return;
  }
  int column_index = m_columns->bucket_for_container(container_index);
  Q_ASSERT(column_index>=0); // Every container should be in both rows and columns, or neither place.
  int row_to_remove = -1;
  int column_to_remove = -1;
  if(m_rows->container_indexes(row_index).size()==1) {
    row_to_remove = row_index;
    emit begin_remove_row(row_index);
  }
  if(m_columns->container_indexes(column_index).size()==1) {
    column_to_remove = column_index;
    emit begin_remove_column(column_index);
  }
  m_columns->remove(container_index);
  m_rows->remove(container_index);
  if(column_to_remove>=0) {
    emit remove_column(column_to_remove);
  }
  if(row_to_remove>=0) {
    emit remove_row(row_to_remove);
  }
  if(row_to_remove==-1 && column_to_remove==-1) {
    emit data_changed(row_index,column_index);
  }
}

void datacube_t::readd(int index) {
  m_columns->readd(index);
  m_rows->readd(index);
  emit changed();
}

}

#include "datacube.moc"

