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
#include <QPainter>
#include "datacube.h"

namespace qdatacube {

class datacube_view_private_t : public QSharedData {
  public:
    datacube_t* datacube;
};

datacube_view_t::datacube_view_t(QWidget* parent):
    QAbstractScrollArea(parent),
    d(new datacube_view_private_t)
{
}

datacube_header_t* datacube_view_t::horizontalHeader() const {
  return 0L;
}

datacube_header_t* datacube_view_t::verticalHeader() const {
  return 0L;
}

void datacube_view_t::set_datacube(datacube_t* datacube) {
  d->datacube = datacube;
  viewport()->update();
  connect(datacube, SIGNAL(reset()), viewport(), SLOT(update()));
  connect(datacube, SIGNAL(data_changed(int,int)), viewport(), SLOT(update()));
}

datacube_view_t::~datacube_view_t() {

}

bool datacube_view_t::viewportEvent(QEvent* event)
{
  if (event->type() == QEvent::Paint) {
    QPaintEvent* paintevent = static_cast<QPaintEvent*>(event);
    paint_datacube(paintevent);
  }
  return QAbstractScrollArea::viewportEvent(event);
}
void datacube_view_t::paint_datacube(QPaintEvent* event) const {
  QPainter painter(viewport());
  QRect rect = event->rect();
//   painter.fillRect(rect, Qt::white);
  QStyleOption options;
  options.initFrom(viewport());
  QSize cell_size(fontMetrics().width("9999"), fontMetrics().lineSpacing());
  options.rect.setSize(cell_size);
  int vertical_header_width = d->datacube->header_count(Qt::Vertical) * cell_size.width();
  int horizontal_header_height = d->datacube->header_count(Qt::Horizontal) * cell_size.height();
  QStyleOption header_options(options);
  painter.setBrush(Qt::green);
  QFont normalfont = painter.font();
  QFont boldfont = painter.font();
  boldfont.setBold(true);
  painter.setFont(boldfont);
  painter.setPen(Qt::white);
  for (int hh = 0, hh_count = d->datacube->header_count(Qt::Horizontal); hh < hh_count; ++hh) {
    header_options.rect.moveLeft(viewport()->rect().left() + vertical_header_width);
    QList<QPair<QString, int> > headers = d->datacube->headers(Qt::Horizontal, hh);
    for (int header_index = 0; header_index < headers.size(); ++header_index) {
      header_options.rect.setSize(QSize(cell_size.width()*headers[header_index].second, cell_size.height()));
      painter.setPen(Qt::black);
      painter.drawRect(header_options.rect);
      painter.setPen(Qt::white);
      painter.drawText(header_options.rect.adjusted(1,1,-2,-2), Qt::AlignCenter, headers[header_index].first);
      header_options.rect.translate(header_options.rect.width(), 0);
    }
    header_options.rect.translate(0, cell_size.height());
  }
  header_options.rect.moveLeft(viewport()->rect().left());
  options.rect.moveTop(header_options.rect.top());
  for (int vh = 0, vh_count = d->datacube->header_count(Qt::Vertical); vh < vh_count; ++vh) {
    header_options.rect.moveTop(options.rect.top());
    QList<QPair<QString, int> > headers = d->datacube->headers(Qt::Vertical, vh);
    for (int header_index = 0; header_index < headers.size(); ++header_index) {
      header_options.rect.setSize(QSize(cell_size.width(), cell_size.height()*headers[header_index].second));
      painter.setPen(Qt::black);
      painter.drawRect(header_options.rect);
      painter.setPen(Qt::white);
      painter.drawText(header_options.rect.adjusted(1,1,-2,-2), Qt::AlignCenter, headers[header_index].first);
      header_options.rect.translate(0, header_options.rect.height());
    }
    header_options.rect.translate(cell_size.width(),0);
  }
  painter.setBrush(QBrush());
  painter.setPen(QPen());
  painter.setFont(normalfont);
  for (int r = 0, nr=d->datacube->row_count(); r<nr; ++r) {
    options.rect.moveLeft(viewport()->rect().left() + vertical_header_width);
    for (int c = 0, nc=d->datacube->column_count(); c<nc; ++c) {
      style()->drawItemText(&painter, options.rect, Qt::AlignCenter, palette(), true, QString::number(d->datacube->element_count(r,c)));
      painter.drawRect(options.rect);
      options.rect.translate(cell_size.width(),0);
    }
    options.rect.translate(0,cell_size.height());
  }



  event->setAccepted(true);
}



} // end of namespace

#include "datacube_view.moc"
