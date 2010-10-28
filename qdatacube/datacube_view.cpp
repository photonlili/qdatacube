/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_view.h"
#include "datacube_selection_model.h"
#include "datacube_view_item_delegate.h"
#include <QtGui/QHeaderView>
#include <QtGui/QMouseEvent>
#include "datacube_header.h"

namespace qdatacube {

class datacube_view_private_t : public QSharedData {
};

datacube_view_t::datacube_view_t(QWidget* parent): QTableView(parent) {
  datacube_header_t* hozheader = new datacube_header_t(Qt::Horizontal,  this);
  datacube_header_t* verheader = new datacube_header_t(Qt::Vertical,  this);
  setHorizontalHeader(hozheader);
  setVerticalHeader(verheader);
}

void datacube_view_t::setSelectionModel(QItemSelectionModel* selectionModel) {
  QTableView::setSelectionModel(selectionModel);
}

void datacube_view_t::setSelectionModel(datacube_selection_model_t* selection_model) {
  connect(selection_model, SIGNAL(partial_selection_changed(QItemSelection,QItemSelection)), SLOT(selectionChanged(QItemSelection,QItemSelection)));
  QTableView::setSelectionModel(selection_model);
  datacube_view_item_delegate_t* delegate = new datacube_view_item_delegate_t(selection_model, selection_model);
  setItemDelegate(delegate);

}
datacube_header_t* datacube_view_t::horizontalHeader() const {
  return qobject_cast<datacube_header_t*>(QTableView::horizontalHeader());
}
void datacube_view_t::setHorizontalHeader(qdatacube::datacube_header_t* headerview)
{
  QTableView::setHorizontalHeader(headerview);
}
void datacube_view_t::setVerticalHeader(qdatacube::datacube_header_t* headerview
) {
  QTableView::setVerticalHeader(headerview);
}

datacube_header_t* datacube_view_t::verticalHeader() const {
  return qobject_cast<datacube_header_t*>(QTableView::verticalHeader());
}

datacube_view_t::~datacube_view_t() {

}

} // end of namespace

#include "datacube_view.moc"
