/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_model.h"
#include "datacube.h"
#include "datacube_colrow.h"

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

datacube_model_t::datacube_model_t(datacube_t* datacube, QObject* parent) :
    QAbstractTableModel(parent),
    m_datacube(datacube)
{
  connect(m_datacube, SIGNAL(changed()), SLOT(slot_qdatacube_changed()));
  connect(m_datacube,SIGNAL(begin_remove_column(int)), SLOT(slot_begin_remove_column(int)));
  connect(m_datacube,SIGNAL(begin_remove_row(int)), SLOT(slot_begin_remove_row(int)));
  connect(m_datacube,SIGNAL(remove_column(int)), SLOT(slot_remove_column(int)));
  connect(m_datacube,SIGNAL(remove_row(int)),SLOT(slot_remove_row(int)));
  connect(m_datacube,SIGNAL(data_changed(int,int)),SLOT(slot_data_changed(int,int)));
}

datacube_model_t::~datacube_model_t() {
}

QVariant datacube_model_t::data(const QModelIndex& index, int role) const {
  if (role == Qt::DisplayRole) {
    return m_datacube->cellCount(index.row(), index.column());
  }
  return QVariant();
}

int datacube_model_t::columnCount(const QModelIndex& ) const {
  return m_datacube->columnCount();
}

int datacube_model_t::rowCount(const QModelIndex& ) const {
  return m_datacube->rowCount();
}

QVariant datacube_model_t::headerData(int section, Qt::Orientation orientation, int role) const {
  return QAbstractItemModel::headerData(section, orientation, role);
}


void datacube_model_t::slot_datacube_changed() {
  reset();
  emit changed();
}

void datacube_model_t::remove(int container_index) {
  m_datacube->remove(container_index);
}

void datacube_model_t::readd(int container_index) {
  m_datacube->readd(container_index);
}

void datacube_model_t::restrict_to_indexes(QList< int > list) {
  m_datacube->restrict(list);
}

void datacube_model_t::slot_begin_remove_column(int column ) {
  beginRemoveColumns(QModelIndex(),column,column);
}

void datacube_model_t::slot_begin_remove_row(int row ){
  beginRemoveRows(QModelIndex(),row,row);
}

void datacube_model_t::slot_remove_column(int ) {
  endRemoveColumns();
}
void datacube_model_t::slot_remove_row(int ){
  endRemoveRows();
}
void datacube_model_t::slot_data_changed(int row , int column ) {
  dataChanged(index(row,column),index(row,column));
}

}

#include "datacube_model.moc"
