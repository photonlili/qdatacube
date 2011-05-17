/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef DATACUBE_VIEW_H
#define DATACUBE_VIEW_H

#include <QtGui/QAbstractScrollArea>
#include "qdatacube_export.h"

namespace qdatacube {

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
     */
    void set_datacube(datacube_t* datacube);

    /**
     * @return current datacube
     */
    datacube_t* datacube() const;
  Q_SIGNALS:
    void vertical_header_context_menu(QPoint pos,int level,int section);
    void horizontal_header_context_menu(QPoint pos, int level, int section);
    void cell_context_menu(QPoint pos, int row, int column);
  private Q_SLOTS:
    void relayout();
  protected:
    virtual bool viewportEvent(QEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* event);
  private:
    void paint_datacube(QPaintEvent* event) const;
    QSharedDataPointer<datacube_view_private_t> d;

};

}
#endif // DATACUBE_VIEW_H
