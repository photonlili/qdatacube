/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_view_item_delegate.h"
#include "datacube_model.h"
#include "datacube_selection_model.h"

namespace qdatacube {

struct datacube_view_item_delegate_private_t : QSharedData {
  datacube_view_item_delegate_private_t(datacube_selection_model_t* selection_model) :
     selection_model(selection_model)
  {
  }
  datacube_selection_model_t* selection_model;
};

datacube_view_item_delegate_t::datacube_view_item_delegate_t(datacube_selection_model_t* selection_model, QObject* parent):
    QStyledItemDelegate(parent),
    d(new datacube_view_item_delegate_private_t(selection_model))
{

}

void datacube_view_item_delegate_t::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if (!option.state.testFlag(QStyle::State_Selected) && d->selection_model->partially_selected(index)) {
    // Partially selected
    QStyleOptionViewItemV4 myoption = option;
    myoption.state |= QStyle::State_Selected;
    myoption.palette.setColor(QPalette::Highlight, Qt::red);
    QStyledItemDelegate::paint(painter, myoption, index);
  } else {
    QStyledItemDelegate::paint(painter, option, index);
  }
}

datacube_view_item_delegate_t::~datacube_view_item_delegate_t() {

}

} // end of namespace
