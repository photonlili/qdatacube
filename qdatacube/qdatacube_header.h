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

class qdatacube_t;

class QDATACUBE_EXPORT qdatacube_header_t : public QHeaderView {
  Q_OBJECT
  public:
    qdatacube_header_t(Qt::Orientation orientation, QWidget* parent = 0);
    virtual QSize sizeHint() const;
    virtual void setModel(QAbstractItemModel* model);
  protected:
    virtual void paintEvent(QPaintEvent* e);
    virtual int sizeHintForColumn(int column) const;
    virtual int sizeHintForRow(int row) const;

  public slots:
    void setAllOffset(int value);
    void updateSizes();
    void setSectionSize(int section, int oldsize, int size);
    void slotSectionResized(int section, int oldsize, int size);
    virtual void reset();

  private:
    QSize m_size;
    QList<QHeaderView*> m_headers;
    bool m_lock;
    QLayout* m_layout;
    /**
     * Helper function to partition the size changed (oldsize-size) into
     * span integer pieces. This function will fetch the i'th piece.
     * The essential property of this function is that given a larger number
     * of resizes, the size changed should be evenly distributed across the
     * pieces
     */
    int getnewlhsize(int oldsize, int size, int i, int span);

};

}
#endif // RECAP_HEADER_H
