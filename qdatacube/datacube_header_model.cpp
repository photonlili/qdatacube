/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_header_model.h"
#include "datacube_model.h"
#include "datacube.h"

namespace qdatacube {
class datacube_header_model_t::secret_t {
  public:
    Qt::Orientation orientation;
    datacube_model_t* datacube_model;
    int index;
    QList<QPair<QString, int> > header_data;

    secret_t(datacube_model_t* qdatacube_model, Qt::Orientation orientation, int index);
};

datacube_header_model_t::secret_t::secret_t(datacube_model_t* datacube_model,
                                            Qt::Orientation orientation,
                                            int index) :
    orientation(orientation),
    datacube_model(datacube_model),
    index(index),
    header_data(datacube_model->datacube()->headers(orientation, index))
{

}


datacube_header_model_t::datacube_header_model_t(datacube_model_t* qdatacube_model, Qt::Orientation orientation, int index):
    QAbstractItemModel(qdatacube_model),
    d(new secret_t(qdatacube_model, orientation, index))
{
  if(index==0) {
    if(d->orientation == Qt::Horizontal) {
      connect(d->datacube_model,SIGNAL(columnsRemoved(const QModelIndex&,int,int)),SLOT(reset_headers()));
      connect(d->datacube_model,SIGNAL(columnsInserted(const QModelIndex&,int,int)),SLOT(reset_headers()));
    } else {
      connect(d->datacube_model,SIGNAL(rowsRemoved(const QModelIndex&,int,int)),SLOT(reset_headers()));
      connect(d->datacube_model,SIGNAL(rowsInserted(const QModelIndex&,int,int)),SLOT(reset_headers()));
    }
  }
  connect(d->datacube_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),SLOT(reset_headers()));
  connect(d->datacube_model, SIGNAL(modelReset()), SLOT(reset_headers()));
}

int datacube_header_model_t::columnCount(const QModelIndex& ) const {
  return d->orientation == Qt::Horizontal ? d->header_data.size() : 0;
}

int datacube_header_model_t::rowCount(const QModelIndex& ) const {
  return d->orientation == Qt::Vertical ? d->header_data.size() : 0;

}

QVariant datacube_header_model_t::headerData(int section, Qt::Orientation orientation, int role) const {
  switch(role) {
    case Qt::DisplayRole:
      return d->header_data[section].first;
    case Qt::UserRole+1:
      return d->header_data[section].second;
    default:
      return QAbstractItemModel::headerData(section, orientation, role);
  }
}

void datacube_header_model_t::reset_headers() {
  reset();
}
}

#include "datacube_header_model.moc"