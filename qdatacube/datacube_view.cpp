/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_view.h"
#include <QtGui/QHeaderView>
#include <QtGui/QMouseEvent>
#include <QPainter>
#include "datacube.h"
#include "datacube_selection.h"
#include "cell.h"
#include <QScrollBar>
#include "abstract_aggregator.h"
#include "abstract_filter.h"

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
    QSize visible_cells;
    QPoint mouse_press_point;
    QRect selection_area;
    QRect header_selection_area;
    bool show_totals;

    /**
     *  Return cell corresponding to position. Note that position is zero-based, and if no cells at position an
     * invalid cell_t is returned (i.e., cell_for_position(outside_pos).invalid() == true );
     **/
    cell_t cell_for_position(QPoint pos, int vertical_scrollbar_value, int horizontal_scrollbar_value) const;
};

datacube_view_private_t::datacube_view_private_t()
    : datacube(0L),
    selection(0L),
    horizontal_header_height(-1),
    vertical_header_width(-1),
    cell_size(),
    datacube_size(),
    show_totals(true)
{

}

cell_t datacube_view_private_t::cell_for_position(QPoint pos, int vertical_scrollbar_value, int horizontal_scrollbar_value) const {
  const int ncolumn_headers = datacube->header_count(Qt::Horizontal);
  const int nrow_headers = datacube->header_count(Qt::Vertical);
  const int column = (pos.x() / cell_size.width()) - nrow_headers;
  const int row = (pos.y() / cell_size.height()) - ncolumn_headers;
  const int nrows = datacube_size.height() + (show_totals ? datacube->header_count(Qt::Horizontal) : 0);
  const int ncolumns = datacube_size.width() + (show_totals ? datacube->header_count(Qt::Vertical) : 0);
  if (0 <= row && 0 <= column && nrows && column < ncolumns && (row <= datacube_size.height() || column < datacube_size.width())) {
    return cell_t(row + vertical_scrollbar_value, column + horizontal_scrollbar_value);
  } else if (row < 0 && -row <= datacube->header_count(Qt::Horizontal)) {
    if (column>=0 && column < ncolumns) {
      // row headers
      return cell_t(row, column + horizontal_scrollbar_value);
    } else if (column < 0 && -column <= datacube->header_count(Qt::Vertical)) {
      return cell_t(row, column);
    }
  } else if (column < 0 && -column <= datacube->header_count(Qt::Vertical) && row>=0 && row < nrows) {
    return cell_t(row + vertical_scrollbar_value, column);
  }
  return cell_t();
}


datacube_view_t::datacube_view_t(QWidget* parent):
    QAbstractScrollArea(parent),
    d(new datacube_view_private_t) {
}

void datacube_view_t::set_datacube(datacube_t* datacube) {
  if (d->datacube) {
    d->datacube->disconnect(this);
  }
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
  connect(datacube, SIGNAL(global_filter_changed()), SLOT(relayout()));
  connect(datacube, SIGNAL(destroyed(QObject*)), SLOT(deleteLater()));
  relayout();
}

void datacube_view_t::relayout() {
  d->datacube_size = QSize(d->datacube->column_count(), d->datacube->row_count());
  d->cell_size = QSize(fontMetrics().width("9999"), fontMetrics().lineSpacing());
  d->vertical_header_width = qMax(1, d->datacube->header_count(Qt::Vertical)) * d->cell_size.width();
  d->horizontal_header_height = qMax(1, d->datacube->header_count(Qt::Horizontal)) * d->cell_size.height();
  QSize visible_size = viewport()->size();
  // Calculate the number of rows and columns at least partly visible
  const int rows_visible = (visible_size.height() - d->horizontal_header_height + 1) / (d->cell_size.height());
  const int columns_visible = (visible_size.width() - d->vertical_header_width + 1) / (d->cell_size.width());
  const int nrows = d->datacube_size.height() + (d->show_totals ? d->datacube->header_count(Qt::Horizontal) : 0);
  const int ncolumns = d->datacube_size.width() + (d->show_totals ? d->datacube->header_count(Qt::Vertical) : 0);
  d->visible_cells = QSize(qMin(columns_visible, ncolumns), qMin(rows_visible, nrows));
  // Set range of scrollbars
  verticalScrollBar()->setRange(0, qMax(0, nrows - rows_visible));
  verticalScrollBar()->setPageStep(rows_visible);
  horizontalScrollBar()->setRange(0, qMax(0, ncolumns - columns_visible));
  horizontalScrollBar()->setPageStep(columns_visible);
  viewport()->update();
}

void datacube_view_t::resizeEvent(QResizeEvent* event)
{
  QAbstractScrollArea::resizeEvent(event);
  relayout();
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
  int horizontal_header_height = d->horizontal_header_height;
  QRect header_rect(options.rect);
  painter.setBrush(palette().button());
  painter.setPen(palette().color(QPalette::WindowText));

  // Draw global filter corner, if applicable
  QRect cornerRect(options.rect.topLeft(), QSize(d->vertical_header_width,d->horizontal_header_height));
  painter.drawRect(cornerRect);
  painter.setPen(palette().buttonText().color());
  if (std::tr1::shared_ptr<abstract_filter_t> global_filter = datacube()->global_filters().value(0)) {
    QString global_category = global_filter->short_name();
    painter.drawText(cornerRect.adjusted(1, 1, -2, -2), Qt::AlignCenter, global_category);
  }


  // Draw horizontal header
  const int leftmost_column = horizontalScrollBar()->value();
  const int rightmost_column = leftmost_column + d->visible_cells.width();
  const int topmost_column = verticalScrollBar()->value();
  const int bottommost_column = topmost_column + d->visible_cells.height();
  const int horizontal_header_count = d->datacube->header_count(Qt::Horizontal);
  const int ndatarows = d->datacube->row_count();
  QRect summary_rect(header_rect);
  summary_rect.translate(0, d->cell_size.height()*(ndatarows-topmost_column+horizontal_header_count));
  for (int hh = 0; hh < horizontal_header_count; ++hh) {
    header_rect.moveLeft(viewport()->rect().left() + vertical_header_width);
    summary_rect.moveLeft(header_rect.left());
    QList<QPair<QString, int> > headers = d->datacube->headers(Qt::Horizontal, hh);
    int current_cell_equivalent = 0;
    for (int header_index = 0; header_index < headers.size() && current_cell_equivalent <= rightmost_column; ++header_index) {
      painter.setBrush((d->header_selection_area.contains(-hh-1, header_index)) ? palette().highlight() : palette().button());
      int header_span = headers[header_index].second;
      current_cell_equivalent += header_span;
      if (current_cell_equivalent < leftmost_column) {
        continue;
      }
      if (current_cell_equivalent > rightmost_column) {
        header_span -= (current_cell_equivalent - rightmost_column - 1);
      }
      if (current_cell_equivalent - header_span < leftmost_column) {
        // Force header cell to available width, for maximum readability
        header_span = (current_cell_equivalent - leftmost_column);
      }
      header_rect.setSize(QSize(cell_size.width()*header_span, cell_size.height()));
      painter.drawRect(header_rect);
      painter.drawText(header_rect.adjusted(1, 1, -1, -1), Qt::AlignCenter, headers[header_index].first);
      header_rect.translate(header_rect.width(), 0);
      if (d->show_totals && bottommost_column > hh + ndatarows) {
        summary_rect.setSize(header_rect.size());
        painter.drawRect(summary_rect);
        painter.drawText(summary_rect.adjusted(1,1,-1,-1), Qt::AlignCenter, QString::number(d->datacube->element_count(Qt::Horizontal, hh, header_index)));
        summary_rect.translate(summary_rect.width(), 0);
      }
    }
    header_rect.translate(0, cell_size.height());
    summary_rect.translate(0, cell_size.height());
  }
  if (horizontal_header_count == 0) {
    header_rect.moveLeft(viewport()->rect().left() + vertical_header_width);
    header_rect.setSize(QSize(cell_size.width(), cell_size.height()));
    painter.drawRect(header_rect);
  }

  // Draw vertical header
  const int ndatacolumns = d->datacube->column_count();
  const int vertical_header_count = d->datacube->header_count(Qt::Vertical);
  header_rect.moveLeft(viewport()->rect().left());
  summary_rect.moveLeft(header_rect.left() + d->cell_size.width()*(ndatacolumns-leftmost_column+vertical_header_count));
  options.rect.moveTop(viewport()->rect().top() + horizontal_header_height);

  for (int vh = 0; vh < vertical_header_count; ++vh) {
    header_rect.moveTop(options.rect.top());
    summary_rect.moveTop(header_rect.top());
    QList<QPair<QString, int> > headers = d->datacube->headers(Qt::Vertical, vh);
    int current_cell_equivalent = 0;
    for (int header_index = 0; header_index < headers.size() && current_cell_equivalent <= bottommost_column; ++header_index) {
      painter.setBrush((d->header_selection_area.contains(header_index, -vh-1)) ? palette().highlight() : palette().button());
      int header_span = headers[header_index].second;
      current_cell_equivalent += header_span;
      if (current_cell_equivalent < topmost_column) {
        continue; // Outside viewport
      }
      if (current_cell_equivalent - header_span < topmost_column) {
        // Force header cell to available width, for maximum readability
        header_span = (current_cell_equivalent - topmost_column);
      }
      if (current_cell_equivalent > bottommost_column) {
        header_span -= (current_cell_equivalent - bottommost_column - 1);
      }
      header_rect.setSize(QSize(cell_size.width(), cell_size.height()*header_span));
      painter.drawRect(header_rect);
      painter.drawText(header_rect.adjusted(1, 1, -2, -2), Qt::AlignCenter, headers[header_index].first);
      header_rect.translate(0, header_rect.height());
      if (d->show_totals && rightmost_column > vh + ndatacolumns) {
        summary_rect.setSize(header_rect.size());
        painter.drawRect(summary_rect);
        painter.drawText(summary_rect.adjusted(1,1,-1,-1), Qt::AlignCenter, QString::number(d->datacube->element_count(Qt::Vertical, vh, header_index)));
        summary_rect.translate(0, summary_rect.height());
      }
    }
    header_rect.translate(cell_size.width(), 0);
    summary_rect.translate(cell_size.width(), 0);
  }
  if (vertical_header_count == 0) {
    header_rect.moveTop(viewport()->rect().top() + horizontal_header_height);
    header_rect.setSize(QSize(cell_size.width(), cell_size.height()));
    painter.drawRect(header_rect);
  }

  painter.setBrush(QBrush());
  painter.setPen(QPen());

  // Draw cells
  QBrush highlight = palette().highlight();
  QColor highligh_color = palette().color(QPalette::Highlight);
  QColor background_color = palette().color(QPalette::Window);
  QBrush faded_highlight = QColor((highligh_color.red() + background_color.red())/2,
                                   (highligh_color.green() + background_color.green())/2,
                                   (highligh_color.blue() + background_color.blue())/2);
  options.rect.moveTop(viewport()->rect().top() + horizontal_header_height);
  for (int r = verticalScrollBar()->value(), nr = d->datacube->row_count(); r < nr; ++r) {
    options.rect.moveLeft(viewport()->rect().left() + vertical_header_width);
    for (int c =horizontalScrollBar()->value(), nc = d->datacube->column_count(); c < nc; ++c) {
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
  int column = (pos.x() - d->vertical_header_width) / d->cell_size.width() + horizontalScrollBar()->value();
  int row = (pos.y() - d->horizontal_header_height) / d->cell_size.height() + verticalScrollBar()->value();
  if (pos.y() < d->horizontal_header_height) {
    if (pos.x() >= d->vertical_header_width) {
      // Hit horizontal headers
      int level = d->datacube->header_count(Qt::Horizontal) - (d->horizontal_header_height - pos.y()) / d->cell_size.height() - 1;
      typedef QPair<QString, int>  headers_t;
      int c = 0;
      int section = 0;
      if (level>=0) {
        Q_FOREACH(headers_t header, d->datacube->headers(Qt::Horizontal, level)) {
          if (c + header.second <= column) {
            ++section;
            c += header.second;
          } else {
            break;
          }
        }
      }
      c -= d->cell_size.width() * horizontalScrollBar()->value(); // Account for scrolled off headers
      QPoint header_element_pos = pos - QPoint(d->vertical_header_width + d->cell_size.width() * c, d->cell_size.height() * level);
      emit horizontal_header_context_menu(header_element_pos, level, section);
    } else {
      emit corner_context_menu(pos);
    }
  } else {
    if (pos.x() < d->vertical_header_width) {
      typedef QPair<QString, int>  headers_t;
      int r = 0;
      int section = 0;
      int level = d->datacube->header_count(Qt::Vertical) - (d->vertical_header_width - pos.x()) / d->cell_size.width() - 1;
      if (level>=0) {
        Q_FOREACH(headers_t header, d->datacube->headers(Qt::Vertical, level)) {
          if (r + header.second <= row) {
            ++section;
            r += header.second;
          } else {
            break;
          }
        }
      }
      r -= d->cell_size.height() * verticalScrollBar()->value(); // Account for scrolled off headers
      QPoint header_element_pos = pos - QPoint(d->cell_size.width() * level, d->horizontal_header_height + d->cell_size.height() * r);
      emit vertical_header_context_menu(header_element_pos, level, section);
    } else {
      // cells
      QPoint cell_pos = pos - QPoint(d->vertical_header_width + ( column - horizontalScrollBar()->value() ) * d->cell_size.width(),
                                     d->horizontal_header_height + ( row - verticalScrollBar()->value() ) * d->cell_size.height());
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
  cell_t press = d->cell_for_position(pos, verticalScrollBar()->value(), horizontalScrollBar()->value());
  if (!(event->modifiers() & Qt::CTRL)) {
    d->selection->clear();
  }
  const int vertical_header_count = d->datacube->header_count(Qt::Vertical);
  const int horizontal_header_count = d->datacube->header_count(Qt::Horizontal);
  if (press.invalid()) {
    d->selection_area = QRect();
  } else if (press.row()>=0 && press.row() < d->datacube_size.height()) {
    if (press.column()>=0 && press.row() < d->datacube_size.width()) {
      d->selection_area = QRect(press.row(), press.column(), 1, 1);
      d->header_selection_area = QRect();
    } else if ((press.column()<0 && -press.column() <= vertical_header_count) || press.column() >= d->datacube_size.width()) {
      const int headerno = press.column() < 0 ? vertical_header_count+press.column() : (press.column() - d->datacube_size.width());
      const int header_section = d->datacube->to_header_section(Qt::Vertical, headerno, press.row());
      QPair<int,int> row_range = d->datacube->to_section(Qt::Vertical, headerno, header_section);
      d->selection_area = QRect(row_range.first, 0, row_range.second - row_range.first + 1, d->datacube_size.width());
      d->header_selection_area = QRect(header_section, -headerno-1, 1,1);
    }
  } else if (press.column()>=0 && press.column() < d->datacube_size.width()) {
    if ((press.row()<0 && -press.row() <= horizontal_header_count) || press.row() >= d->datacube_size.height()) {
      const int headerno = press.row() < 0 ? horizontal_header_count+press.row() : (press.row() - d->datacube_size.height());
      const int header_section = d->datacube->to_header_section(Qt::Horizontal, headerno, press.column());
      QPair<int,int> column_range = d->datacube->to_section(Qt::Horizontal, headerno, header_section);
      d->selection_area = QRect(0, column_range.first, d->datacube_size.height(), column_range.second - column_range.first + 1);
      d->header_selection_area = QRect(-headerno-1, header_section, 1,1);

    }
  }
  viewport()->update();
  event->accept();
}

void datacube_view_t::mouseMoveEvent(QMouseEvent* event) {
  QAbstractScrollArea::mouseMoveEvent(event);
  cell_t press = d->cell_for_position(d->mouse_press_point, verticalScrollBar()->value(), horizontalScrollBar()->value());
  QPoint constrained_pos = QPoint(qMax(1, qMin(event->pos().x(), d->vertical_header_width + d->datacube_size.width() * d->cell_size.width() + (d->show_totals ? d->vertical_header_width : 0) -1)),
                                  qMax(1, qMin(event->pos().y(), d->horizontal_header_height + d->datacube_size.height() * d->cell_size.height() + (d->show_totals ? d->horizontal_header_height : 0) - 1)));
  cell_t current = d->cell_for_position(constrained_pos,verticalScrollBar()->value(), horizontalScrollBar()->value());
  QRect new_selection_area;
  if (!press.invalid() && !current.invalid()) {
    const int vertical_header_count = d->datacube->header_count(Qt::Vertical);
    const int horizontal_header_count = d->datacube->header_count(Qt::Horizontal);
    if (0<=press.row() && press.row() < d->datacube_size.height()) {
      if (0<=press.column() && press.column() < d->datacube_size.width()) {
        const int upper_row = qMin(press.row(), current.row());
        const int height = qAbs(press.row() - current.row()) + 1;
        const int left_column = qMin(press.column(), current.column());
        const int width = qAbs(press.column() - current.column()) + 1;
        new_selection_area = QRect(upper_row, left_column, height, width);
        d->header_selection_area = QRect();
      } else if (((vertical_header_count <= press.column() && press.column()) <= 0) || press.column() >= d->datacube_size.width()) {
        const int headerno = press.column() < 0 ? vertical_header_count+press.column() : (press.column() - d->datacube_size.width());
        int upper_header_section = d->datacube->to_header_section(Qt::Vertical, headerno, press.row());
        int lower_header_section = d->datacube->to_header_section(Qt::Vertical, headerno, qMax(0, qMin(current.row(), d->datacube_size.height()-1)));
        if (upper_header_section > lower_header_section) {
          std::swap(upper_header_section, lower_header_section);
        }
        const int upper_row = d->datacube->to_section(Qt::Vertical, headerno, upper_header_section).first;
        const int lower_row = d->datacube->to_section(Qt::Vertical, headerno, lower_header_section).second;
        const int left_column = 0;
        const int width = d->datacube_size.width();
        new_selection_area = QRect(upper_row, left_column, lower_row-upper_row+1, width);
        d->header_selection_area = QRect(upper_header_section, -headerno-1, lower_header_section-upper_header_section+1, 1);
      }
    } else if (press.column()>=0 && press.column() < d->datacube_size.width()) {
      if ((press.row()<0 && -press.row() <= horizontal_header_count) || press.row() >= d->datacube_size.height()) {
        const int headerno = press.row() < 0 ? horizontal_header_count+press.row() : (press.row() - d->datacube_size.height());
        int leftmost_header_section = d->datacube->to_header_section(Qt::Horizontal, headerno, press.column());
        int rightmost_header_section = d->datacube->to_header_section(Qt::Horizontal, headerno, qMax(0, qMin(current.column(), d->datacube_size.width()-1)));
        if (leftmost_header_section > rightmost_header_section) {
          std::swap(leftmost_header_section, rightmost_header_section);
        }
        const int leftmost_column = d->datacube->to_section(Qt::Horizontal, headerno, leftmost_header_section).first;
        const int rightmost_column = d->datacube->to_section(Qt::Horizontal, headerno, rightmost_header_section).second;
        const int upper_row = 0;
        const int height = d->datacube_size.height();
        new_selection_area = QRect(upper_row, leftmost_column, height, rightmost_column-leftmost_column+1);
        d->header_selection_area = QRect(-headerno-1, leftmost_header_section, 1, rightmost_header_section-leftmost_header_section+1);
      }
    }
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
        if (r >= 0 && c >= 0 && r < d->datacube_size.height() && c < d->datacube_size.width()) {
          d->selection->add_cell(r, c);
        } else {
          if (c >= d->datacube_size.width()) {
            //
          } else if (r>= d->datacube_size.height()) {
            //
          }

        }
      }
    }
  }
  d->selection_area = QRect();
  d->header_selection_area = QRect();
  d->mouse_press_point = QPoint();
}

datacube_selection_t* datacube_view_t::datacube_selection() const
{
  return d->selection;
}

QRect datacube_view_t::corner() const {
  return QRect(0,0,d->vertical_header_width, d->horizontal_header_height);
}

} // end of namespace

#include "datacube_view.moc"
