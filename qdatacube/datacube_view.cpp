/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_view.h"
#include "datacube_selection_model.h"
#include "datacube_view_item_delegate.h"
#include <QtGui/QHeaderView>
#include <QtGui/QMouseEvent>
#include <QPainter>
#include "datacube.h"
#include "datacube_selection.h"
#include "cell.h"

namespace qdatacube {

class datacube_view_private_t : public QSharedData {
  public:
    datacube_view_private_t();
    datacube_t* datacube;
    datacube_selection_t* selection;
    int horizontal_header_height;
    int vertical_header_width;
    QSize cell_size;
    QSize datacube_size;
    QPoint mouse_press_point;
    QRect selection_area;

    /**
     *  Return cell corresponding to position. Note that position is zero-based, and if no cells at position an
     * invalid cell_t is returned (i.e., cell_for_position(outside_pos).invalid() == true );
     **/
    cell_t cell_for_position(QPoint pos) const;
};

datacube_view_private_t::datacube_view_private_t()
    : datacube(0L),
    selection(0L),
    horizontal_header_height(-1),
    vertical_header_width(-1),
    cell_size(),
    datacube_size() {

}

cell_t datacube_view_private_t::cell_for_position(QPoint pos) const {
  const int column = (pos.x() - vertical_header_width) / cell_size.width();
  const int row = (pos.y() - horizontal_header_height) / cell_size.height();
  if (0 <= row && 0 <= column && row < datacube_size.height() && column < datacube_size.width()) {
    return cell_t(row, column);
  } else {
    return cell_t();
  }
}


datacube_view_t::datacube_view_t(QWidget* parent):
    QAbstractScrollArea(parent),
    d(new datacube_view_private_t) {
}

void datacube_view_t::set_datacube(datacube_t* datacube) {
  d->datacube = datacube;
  delete d->selection;
  d->selection = new datacube_selection_t(datacube, this);
  viewport()->update();
  connect(d->selection, SIGNAL(selection_status_changed(int, int)), viewport(), SLOT(update()));
  connect(datacube, SIGNAL(reset()), SLOT(relayout()));
  connect(datacube, SIGNAL(data_changed(int, int)), viewport(), SLOT(update()));
  connect(datacube, SIGNAL(columns_inserted(int, int)), SLOT(relayout()));
  connect(datacube, SIGNAL(rows_inserted(int, int)), SLOT(relayout()));
  connect(datacube, SIGNAL(columns_removed(int, int)), SLOT(relayout()));
  connect(datacube, SIGNAL(rows_removed(int, int)), SLOT(relayout()));
  relayout();
}

void datacube_view_t::relayout() {
  d->datacube_size = QSize(d->datacube->column_count(), d->datacube->row_count());
  d->cell_size = QSize(fontMetrics().width("9999"), fontMetrics().lineSpacing());
  d->vertical_header_width = d->datacube->header_count(Qt::Vertical) * d->cell_size.width();
  d->horizontal_header_height = d->datacube->header_count(Qt::Horizontal) * d->cell_size.height();
  viewport()->update();
}


datacube_view_t::~datacube_view_t() {

}

bool datacube_view_t::viewportEvent(QEvent* event) {
  if (event->type() == QEvent::Paint) {
    QPaintEvent* paintevent = static_cast<QPaintEvent*>(event);
    paint_datacube(paintevent);
  }
  return QAbstractScrollArea::viewportEvent(event);
}
void datacube_view_t::paint_datacube(QPaintEvent* event) const {
  QPainter painter(viewport());
  QStyleOption options;
  options.initFrom(viewport());
  QSize cell_size = d->cell_size;
  options.rect.setSize(cell_size);
  int vertical_header_width = d->vertical_header_width;
  QStyleOption header_options(options);
  painter.setBrush(Qt::green);
  QFont normalfont = painter.font();
  QFont boldfont = painter.font();
  boldfont.setBold(true);
  painter.setFont(boldfont);
  painter.setPen(Qt::white);

  // Draw horizontal header
  for (int hh = 0, hh_count = d->datacube->header_count(Qt::Horizontal); hh < hh_count; ++hh) {
    header_options.rect.moveLeft(viewport()->rect().left() + vertical_header_width);
    QList<QPair<QString, int> > headers = d->datacube->headers(Qt::Horizontal, hh);
    for (int header_index = 0; header_index < headers.size(); ++header_index) {
      header_options.rect.setSize(QSize(cell_size.width()*headers[header_index].second, cell_size.height()));
      painter.setPen(Qt::black);
      painter.drawRect(header_options.rect);
      painter.setPen(Qt::white);
      painter.drawText(header_options.rect.adjusted(1, 1, -2, -2), Qt::AlignCenter, headers[header_index].first);
      header_options.rect.translate(header_options.rect.width(), 0);
    }
    header_options.rect.translate(0, cell_size.height());
  }
  header_options.rect.moveLeft(viewport()->rect().left());
  options.rect.moveTop(header_options.rect.top());

  // Draw vertical header
  for (int vh = 0, vh_count = d->datacube->header_count(Qt::Vertical); vh < vh_count; ++vh) {
    header_options.rect.moveTop(options.rect.top());
    QList<QPair<QString, int> > headers = d->datacube->headers(Qt::Vertical, vh);
    for (int header_index = 0; header_index < headers.size(); ++header_index) {
      header_options.rect.setSize(QSize(cell_size.width(), cell_size.height()*headers[header_index].second));
      painter.setPen(Qt::black);
      painter.drawRect(header_options.rect);
      painter.setPen(Qt::white);
      painter.drawText(header_options.rect.adjusted(1, 1, -2, -2), Qt::AlignCenter, headers[header_index].first);
      header_options.rect.translate(0, header_options.rect.height());
    }
    header_options.rect.translate(cell_size.width(), 0);
  }
  painter.setBrush(QBrush());
  painter.setPen(QPen());
  painter.setFont(normalfont);

  // Draw cells
  QBrush highlight = palette().highlight();
  QBrush faded_highlight = highlight;
  faded_highlight.setStyle(Qt::CrossPattern);
  for (int r = 0, nr = d->datacube->row_count(); r < nr; ++r) {
    options.rect.moveLeft(viewport()->rect().left() + vertical_header_width);
    for (int c = 0, nc = d->datacube->column_count(); c < nc; ++c) {
      datacube_selection_t::selection_status_t selection_status = d->selection->selection_status(r, c);
      bool highlighted = false;
      switch (selection_status) {
        case datacube_selection_t::UNSELECTED:
          if (d->selection_area.contains(r, c)) {
            painter.fillRect(options.rect, highlight);
            highlighted = true;
          }
          break;
        case datacube_selection_t::SELECTED:
          if (d->selection_area.contains(r, c)) {
            painter.fillRect(options.rect, highlight);
            highlighted = true;
          } else {
            painter.fillRect(options.rect, highlight);
            highlighted = true;
          }
          break;
        case datacube_selection_t::PARTIALLY_SELECTED:
          if (d->selection_area.contains(r, c)) {
            painter.fillRect(options.rect, highlight);
            highlighted = true;
          } else {
            painter.fillRect(options.rect, faded_highlight);
          }
          break;
      }
      if (int item_value = d->datacube->element_count(r, c)) {
        style()->drawItemText(&painter, options.rect, Qt::AlignCenter, palette(), true, QString::number(item_value), highlighted ? QPalette::HighlightedText : QPalette::Text);
      }
      painter.drawRect(options.rect);
      options.rect.translate(cell_size.width(), 0);
    }
    options.rect.translate(0, cell_size.height());
  }

  event->setAccepted(true);
}

void datacube_view_t::contextMenuEvent(QContextMenuEvent* event) {
  QPoint pos = event->pos();
  if (pos.x() >= d->vertical_header_width + d->cell_size.width()*d->datacube->column_count()) {
    event->setAccepted(false);
    return; // Right of datacube
  }
  if (pos.y() >= d->horizontal_header_height + d->cell_size.height()*d->datacube->row_count()) {
    event->setAccepted(false);
    return;
  }
  event->accept();
  int column = (pos.x() - d->vertical_header_width) / d->cell_size.width();
  int row = (pos.y() - d->horizontal_header_height) / d->cell_size.height();
  if (pos.y() < d->horizontal_header_height) {
    if (pos.x() >= d->vertical_header_width) {
      // Hit horizontal headers
      int level = d->datacube->header_count(Qt::Horizontal) - (d->horizontal_header_height - pos.y()) / d->cell_size.height() - 1;
      typedef QPair<QString, int>  headers_t;
      int c = 0;
      int section = 0;
      Q_FOREACH(headers_t header, d->datacube->headers(Qt::Horizontal, level)) {
        if (c + header.second <= column) {
          ++section;
          c += header.second;
        } else {
          break;
        }
      }
      QPoint header_element_pos = pos - QPoint(d->vertical_header_width + d->cell_size.width() * c, d->cell_size.height() * level);
      emit horizontal_header_context_menu(header_element_pos, level, section);
    }
  } else {
    if (pos.x() < d->vertical_header_width) {
      int level = d->datacube->header_count(Qt::Vertical) - (d->vertical_header_width - pos.x()) / d->cell_size.width() - 1;
      typedef QPair<QString, int>  headers_t;
      int r = 0;
      int section = 0;
      Q_FOREACH(headers_t header, d->datacube->headers(Qt::Vertical, level)) {
        if (r + header.second <= row) {
          ++section;
          r += header.second;
        } else {
          break;
        }
      }
      QPoint header_element_pos = pos - QPoint(d->cell_size.width() * level, d->horizontal_header_height + d->cell_size.height() * r);
      emit vertical_header_context_menu(header_element_pos, level, section);
    } else {
      // cells
      QPoint cell_pos = pos - QPoint(d->vertical_header_width + column * d->cell_size.width(), d->horizontal_header_height + row * d->cell_size.height());
      cell_context_menu(cell_pos, row, column);
    }
  }

}

datacube_t* datacube_view_t::datacube() const {
  return d->datacube;
}

void datacube_view_t::mousePressEvent(QMouseEvent* event) {
  QAbstractScrollArea::mousePressEvent(event);
  if (event->button() != Qt::LeftButton) {
    event->setAccepted(false);
    return;
  }
  QPoint pos = event->pos();
  d->mouse_press_point = pos;
  cell_t press = d->cell_for_position(pos);
  if (!(event->modifiers() & Qt::CTRL)) {
    d->selection->clear();
  }
  d->selection_area = press.invalid() ? QRect() : QRect(press.row(), press.column(), 1, 1);
  viewport()->update();
  event->accept();
}

void datacube_view_t::mouseMoveEvent(QMouseEvent* event) {
  QAbstractScrollArea::mouseMoveEvent(event);
  cell_t press = d->cell_for_position(d->mouse_press_point);
  cell_t current = d->cell_for_position(event->pos());
  QRect new_selection_area;
  if (!press.invalid() && !current.invalid()) {
    new_selection_area = QRect(qMin(press.row(), current.row()), qMin(press.column(), current.column()), qAbs(press.row() - current.row()) + 1, qAbs(press.column() - current.column()) + 1);
  }
  if (new_selection_area != d->selection_area) {
    d->selection_area = new_selection_area;
    viewport()->update();
  }

}

void datacube_view_t::mouseReleaseEvent(QMouseEvent* event) {
  QAbstractScrollArea::mouseReleaseEvent(event);
  if (!d->selection_area.isNull()) {
    for (int r = d->selection_area.left(); r <= d->selection_area.right(); ++r) {
      for (int c = d->selection_area.top(); c <= d->selection_area.bottom(); ++c) {
        d->selection->add_cell(r, c);
      }
    }
  }
  d->selection_area = QRect();
  d->mouse_press_point = QPoint();
}

datacube_selection_t* datacube_view_t::datacube_selection() const
{
  return d->selection;
}

} // end of namespace

#include "datacube_view.moc"
