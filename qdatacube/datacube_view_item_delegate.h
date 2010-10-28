/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef Q_DATACUBE_DATACUBE_VIEW_ITEM_DELEGATE_H
#define Q_DATACUBE_DATACUBE_VIEW_ITEM_DELEGATE_H

#include <QtGui/QStyledItemDelegate>

namespace qdatacube {

class datacube_selection_model_t;

class datacube_view_item_delegate_private_t;

class datacube_view_item_delegate_t : public QStyledItemDelegate {
  public:
    datacube_view_item_delegate_t(datacube_selection_model_t* selection_model, QObject* parent = 0);
    virtual ~datacube_view_item_delegate_t();
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  private:
    QSharedDataPointer<datacube_view_item_delegate_private_t> d;


};
} // end of namespace

#endif // QDATACUBE_DATACUBE_VIEW_ITEM_DELEGATE_H
