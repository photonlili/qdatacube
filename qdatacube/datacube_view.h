/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef DATACUBE_VIEW_H
#define DATACUBE_VIEW_H

#include <QAbstractScrollArea>
#include "qdatacube_export.h"

namespace qdatacube {

class abstract_formatter_t;
class datacube_selection_t;
class datacube_t;
class datacube_selection_model_t;
class datacube_view_private_t;

class QDATACUBE_EXPORT datacube_view_t : public QAbstractScrollArea  {
  Q_OBJECT
  public:
    datacube_view_t(QWidget* parent = 0);
    virtual ~datacube_view_t();

    /**
     * Set the datacube to be view
     * If the underlying model has changed, this will clear all the formatters
     */
    void set_datacube(datacube_t* datacube);

    /**
     * @return current datacube
     */
    datacube_t* datacube() const;

    /**
     * @return current datacube_selection
     */
    datacube_selection_t* datacube_selection() const;

    /**
     * Add formatter below each cell
     * Ownership over summarizer will be claimed
     */
    void add_formatter(abstract_formatter_t* formatter);

    /**
     * Take formatter by index, clearning parent
     * @return the formatter taken.
     */
    abstract_formatter_t* take_formatter(int index);

    /**
     * @return all the formatters in current use
     */
    QList<abstract_formatter_t*> formatters() const;
  protected:
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event );
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void resizeEvent(QResizeEvent* event);

    /**
     * @return rectangle describing the corner area
     */
    QRect corner() const;
  Q_SIGNALS:
    void vertical_header_context_menu(QPoint pos,int level,int section);
    void horizontal_header_context_menu(QPoint pos, int level, int section);
    void corner_context_menu(QPoint pos);
    void cell_context_menu(QPoint pos, int row, int column);
  private Q_SLOTS:
    void relayout();
    void datacube_deleted();
  protected:
    virtual bool viewportEvent(QEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* event);
  private:
    void paint_datacube(QPaintEvent* event) const;
    QSharedDataPointer<datacube_view_private_t> d;

};

}
#endif // DATACUBE_VIEW_H
