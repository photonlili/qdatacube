/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_header_model.h"
#include "datacube_model.h"
#include "datacube.h"

namespace qdatacube {

datacube_header_model_t::datacube_header_model_t(datacube_model_t* qdatacube_model, Qt::Orientation orientation, int index):
    QAbstractItemModel(qdatacube_model),
    m_orientation(orientation),
    m_datacube_model(qdatacube_model),
    m_index(index),
    m_header_data(qdatacube_model->datacube()->headers(m_orientation, m_index))
{
  if(index==0) {
    if(m_orientation == Qt::Horizontal) {
      connect(m_datacube_model,SIGNAL(columnsRemoved(const QModelIndex&,int,int)),SLOT(slot_remove(const QModelIndex&, int,int)));
    } else {
      connect(m_datacube_model,SIGNAL(rowsRemoved(const QModelIndex&,int,int)),SLOT(slot_remove(const QModelIndex&, int,int)));
    }
  }
}

int datacube_header_model_t::columnCount(const QModelIndex& ) const {
  return m_orientation == Qt::Horizontal ? m_header_data.size() : 0;
}

int datacube_header_model_t::rowCount(const QModelIndex& ) const {
  return m_orientation == Qt::Vertical ? m_header_data.size() : 0;

}

QVariant datacube_header_model_t::headerData(int section, Qt::Orientation orientation, int role) const {
  switch(role) {
    case Qt::DisplayRole:
      return m_header_data[section].first;
    case Qt::UserRole+1:
      return m_header_data[section].second;
    default:
      return QAbstractItemModel::headerData(section, orientation, role);
  }
}

void datacube_header_model_t::slot_remove(const QModelIndex& , int start, int end) {
  Q_ASSERT(start==end);
  Q_UNUSED(start);
  Q_UNUSED(end);
  reset();
}

}
#include "datacube_header_model.moc"