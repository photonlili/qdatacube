/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "qdatacube_header_model.h"
#include "qdatacube_model.h"
#include "qdatacube.h"

namespace qdatacube {

qdatacube_header_model_t::qdatacube_header_model_t(qdatacube_model_t* qdatacube_model, Qt::Orientation orientation, int index):
    QAbstractItemModel(qdatacube_model),
    m_orientation(orientation),
    m_qdatacube_model(qdatacube_model),
    m_index(index),
    m_header_data(qdatacube_model->qdatacube()->headers(m_orientation, m_index))
{
  if(index==0) {
    if(m_orientation == Qt::Horizontal) {
      connect(m_qdatacube_model,SIGNAL(columnsRemoved(const QModelIndex&,int,int)),SLOT(slot_remove(const QModelIndex&, int,int)));
    } else {
      connect(m_qdatacube_model,SIGNAL(rowsRemoved(const QModelIndex&,int,int)),SLOT(slot_remove(const QModelIndex&, int,int)));
    }
  }
}

int qdatacube_header_model_t::columnCount(const QModelIndex& ) const {
  return m_orientation == Qt::Horizontal ? m_header_data.size() : 0;
}

int qdatacube_header_model_t::rowCount(const QModelIndex& ) const {
  return m_orientation == Qt::Vertical ? m_header_data.size() : 0;

}

QVariant qdatacube_header_model_t::headerData(int section, Qt::Orientation orientation, int role) const {
  switch(role) {
    case Qt::DisplayRole:
      return m_header_data[section].first;
    case Qt::UserRole+1:
      return m_header_data[section].second;
    default:
      return QAbstractItemModel::headerData(section, orientation, role);
  }
}

void qdatacube_header_model_t::slot_remove(const QModelIndex& , int start, int end) {
  Q_ASSERT(start==end);
  Q_UNUSED(start);
  Q_UNUSED(end);
  reset();
}

}
#include "qdatacube_header_model.moc"