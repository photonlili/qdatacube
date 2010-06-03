/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "qdatacube_model.h"
#include "qdatacube.h"
#include "qdatacube_colrow.h"

namespace qdatacube {

namespace {
  QList<int> all_rows_from_model(QAbstractItemModel* model) {
    QList<int> rv;
    for (int i=0,n=model->rowCount(); i<n; ++i) {
      rv << i;
    }
    return rv;
  }
}

qdatacube_model_t::qdatacube_model_t(qdatacube_t* datacube, QObject* parent) :
    QAbstractTableModel(parent),
    m_qdatacube(datacube)
{
  connect(m_qdatacube, SIGNAL(changed()), SLOT(slot_qdatacube_changed()));
  connect(m_qdatacube,SIGNAL(begin_remove_column(int)), SLOT(slot_begin_remove_column(int)));
  connect(m_qdatacube,SIGNAL(begin_remove_row(int)), SLOT(slot_begin_remove_row(int)));
  connect(m_qdatacube,SIGNAL(remove_column(int)), SLOT(slot_remove_column(int)));
  connect(m_qdatacube,SIGNAL(remove_row(int)),SLOT(slot_remove_row(int)));
  connect(m_qdatacube,SIGNAL(data_changed(int,int)),SLOT(slot_data_changed(int,int)));
}

qdatacube_model_t::~qdatacube_model_t() {
}

QVariant qdatacube_model_t::data(const QModelIndex& index, int role) const {
  if (role == Qt::DisplayRole) {
    return m_qdatacube->cellCount(index.row(), index.column());
  }
  return QVariant();
}

int qdatacube_model_t::columnCount(const QModelIndex& ) const {
  return m_qdatacube->columnCount();
}

int qdatacube_model_t::rowCount(const QModelIndex& ) const {
  return m_qdatacube->rowCount();
}

QVariant qdatacube_model_t::headerData(int section, Qt::Orientation orientation, int role) const {
  return QAbstractItemModel::headerData(section, orientation, role);
}


void qdatacube_model_t::slot_qdatacube_changed() {
  reset();
  emit changed();
}

void qdatacube_model_t::remove(int container_index) {
  m_qdatacube->remove(container_index);
}

void qdatacube_model_t::readd(int container_index) {
  m_qdatacube->readd(container_index);
}

void qdatacube_model_t::restrict_to_indexes(QList< int > list) {
  m_qdatacube->restrict(list);
}

void qdatacube_model_t::slot_begin_remove_column(int column ) {
  beginRemoveColumns(QModelIndex(),column,column);
}

void qdatacube_model_t::slot_begin_remove_row(int row ){
  beginRemoveRows(QModelIndex(),row,row);
}

void qdatacube_model_t::slot_remove_column(int ) {
  endRemoveColumns();
}
void qdatacube_model_t::slot_remove_row(int ){
  endRemoveRows();
}
void qdatacube_model_t::slot_data_changed(int row , int column ) {
  dataChanged(index(row,column),index(row,column));
}

}

#include "qdatacube_model.moc"
