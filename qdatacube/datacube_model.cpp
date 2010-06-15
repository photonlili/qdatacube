/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_model.h"
#include "datacube.h"
#include "datacube_colrow.h"

namespace qdatacube {

class datacube_model_t::secret_t {
  public:
    datacube_t* datacube;

    secret_t(datacube_t* datacube) : datacube(datacube) {}
};

datacube_model_t::datacube_model_t(datacube_t* datacube, QObject* parent) :
    QAbstractTableModel(parent),
    d(new secret_t(datacube))
{
  connect(d->datacube,SIGNAL(columns_about_to_be_removed(int,int)), SLOT(slot_datacube_about_to_remove_columns(int,int)));
  connect(d->datacube,SIGNAL(rows_about_to_be_removed(int,int)), SLOT(slot_datacube_about_to_remove_rows(int,int)));
  connect(d->datacube,SIGNAL(columns_removed(int,int)), SLOT(slot_datacube_removed_columns(int,int)));
  connect(d->datacube,SIGNAL(rows_removed(int,int)),SLOT(slot_datacube_removed_rows(int,int)));
  connect(d->datacube,SIGNAL(columns_about_to_be_inserted(int,int)), SLOT(slot_datacube_about_to_add_columns(int,int)));
  connect(d->datacube,SIGNAL(rows_about_to_be_inserted(int,int)), SLOT(slot_datacube_about_to_add_rows(int,int)));
  connect(d->datacube,SIGNAL(columns_inserted(int,int)), SLOT(slot_datacube_added_columns(int,int)));
  connect(d->datacube,SIGNAL(rows_inserted(int,int)),SLOT(slot_datacube_added_rows(int,int)));
  connect(d->datacube,SIGNAL(data_changed(int,int)),SLOT(slot_data_changed(int,int)));
  connect(d->datacube,SIGNAL(headers_changed(Qt::Orientation,int,int)), SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
}

datacube_model_t::~datacube_model_t() {
}

QVariant datacube_model_t::data(const QModelIndex& index, int role) const {
  if (index.isValid()) {
    if (role == Qt::DisplayRole) {
      return d->datacube->element_count(index.row(), index.column());
    }
  }
  return QVariant();
}

int datacube_model_t::columnCount(const QModelIndex& ) const {
  return d->datacube->column_count();
}

int datacube_model_t::rowCount(const QModelIndex& ) const {
  return d->datacube->row_count();
}

QVariant datacube_model_t::headerData(int section, Qt::Orientation orientation, int role) const {
  return QAbstractItemModel::headerData(section, orientation, role);
}


void datacube_model_t::slot_datacube_about_to_remove_columns(int column, int count) {
  beginRemoveColumns(QModelIndex(),column,column+count-1);
}

void datacube_model_t::slot_datacube_about_to_remove_rows(int row,int count ){
  beginRemoveRows(QModelIndex(),row,row+count-1);
}

void datacube_model_t::slot_datacube_removed_columns(int,int ) {
  endRemoveColumns();
}

void datacube_model_t::slot_datacube_removed_rows(int,int ){
  endRemoveRows();
}

void datacube_model_t::slot_datacube_about_to_add_columns(int column, int count) {
  beginInsertColumns(QModelIndex(), column, column+count-1);
}

void datacube_model_t::slot_datacube_about_to_add_rows(int row,int count) {
  beginInsertRows(QModelIndex(), row, row+count-1);
}

void datacube_model_t::slot_datacube_added_columns(int,int) {
  endInsertColumns();
}

void datacube_model_t::slot_datacube_added_rows(int,int) {
  endInsertRows();
}

void datacube_model_t::slot_data_changed(int row , int column ) {
  dataChanged(index(row,column),index(row,column));
}

datacube_t* datacube_model_t::datacube() const {
  return d->datacube;
}

}

#include "datacube_model.moc"
