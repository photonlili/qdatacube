/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef RECAP_HEADER_H
#define RECAP_HEADER_H

#include <QHeaderView>

class QLayout;

#include "qdatacube_export.h"

namespace qdatacube {

class datacube_t;

/**
 * This class is a replacement for the standard QHeaderView, to be plugged into QTableViews to
 * show all the headers that a datacube features.
 */
class QDATACUBE_EXPORT datacube_header_t : public QHeaderView {
  Q_OBJECT
  public:
    datacube_header_t(Qt::Orientation orientation, QWidget* parent = 0);
    virtual QSize sizeHint() const;
    virtual void setModel(QAbstractItemModel* model);
  protected:
    virtual void paintEvent(QPaintEvent* e);
    virtual int sizeHintForColumn(int column) const;
    virtual int sizeHintForRow(int row) const;

  Q_SIGNALS:
    /**
     * Emitted when the context menu for a subheader is invoked.
     * -1 means there is no category (probably none is shown)
     */
    void sub_header_context_menu(const QPoint& pos, int headerno, int category);
  public Q_SLOTS:
    void setAllOffset(int value);
    void updateSizes();
    void setSectionSize(int section, int oldsize, int size);
    void slotSectionResized(int section, int oldsize, int size);
    virtual void reset();
  private Q_SLOTS:
    void slot_context_menu_requested(const QPoint& pos);

  private:
    class secret_t;
    QScopedPointer<secret_t> d;

};

}
#endif // RECAP_HEADER_H
