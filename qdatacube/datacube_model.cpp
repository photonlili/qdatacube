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

class datacube_model_t::secret_t {
  public:
    datacube_t* datacube;

    secret_t(datacube_t* datacube) : datacube(datacube) {}
};

datacube_model_t::datacube_model_t(datacube_t* datacube, QObject* parent) :
    QAbstractTableModel(parent),
    d(new secret_t(datacube))
{
  connect(d->datacube, SIGNAL(changed()), SLOT(slot_datacube_changed()));
  connect(d->datacube,SIGNAL(column_about_to_be_removed(int)), SLOT(slot_begin_remove_column(int)));
  connect(d->datacube,SIGNAL(row_about_to_be_removed(int)), SLOT(slot_begin_remove_row(int)));
  connect(d->datacube,SIGNAL(column_removed(int)), SLOT(slot_remove_column(int)));
  connect(d->datacube,SIGNAL(row_removed(int)),SLOT(slot_remove_row(int)));
  connect(d->datacube,SIGNAL(data_changed(int,int)),SLOT(slot_data_changed(int,int)));
}

datacube_model_t::~datacube_model_t() {
}

QVariant datacube_model_t::data(const QModelIndex& index, int role) const {
  if (index.isValid()) {
    if (role == Qt::DisplayRole) {
      return d->datacube->cellCount(index.row(), index.column());
    }
  }
  return QVariant();
}

int datacube_model_t::columnCount(const QModelIndex& ) const {
  return d->datacube->columnCount();
}

int datacube_model_t::rowCount(const QModelIndex& ) const {
  return d->datacube->rowCount();
}

QVariant datacube_model_t::headerData(int section, Qt::Orientation orientation, int role) const {
  return QAbstractItemModel::headerData(section, orientation, role);
}


void datacube_model_t::slot_datacube_changed() {
  reset();
  emit changed();
}

void datacube_model_t::remove(int container_index) {
  d->datacube->remove(container_index);
}

void datacube_model_t::readd(int container_index) {
  d->datacube->readd(container_index);
}

void datacube_model_t::restrict_to_indexes(QList< int > list) {
  d->datacube->restrict(list);
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

datacube_t* datacube_model_t::datacube() const {
  return d->datacube;
}

}

#include "datacube_model.moc"
