/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "internal_header_view.h"

internal_header_view_t::internal_header_view_t(Qt::Orientation orientation, QWidget* parent):
    QHeaderView(orientation, parent)
{

}

QSize internal_header_view_t::minimumSizeHint() const {
  return QSize(0,0);
}

