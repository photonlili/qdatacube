/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef QDATACUBE_QDATACUBE_H
#define QDATACUBE_QDATACUBE_H

#include "qdatacube_export.h"

#include <QObject>
#include <QHash>
#include <memory>

class QAbstractItemModel;

namespace qdatacube {

class abstract_filter_t;
class qdatacube_colrow_t;

class QDATACUBE_EXPORT qdatacube_t : public QObject {
  Q_OBJECT
  public:
    /**
     * Construct simple 2-dimensional qdatacube with the 2 filters
     * @param row_filter initial filter. Ownership is claimed, and filter will be deleted
     * @param column_filter initial filter. Ownership is claimed, and filter will be deleted
     * @param active limited to these containers
     */
    qdatacube_t(const QAbstractItemModel* model,
            abstract_filter_t* row_filter,
            abstract_filter_t* column_filter,
            const QList<int>& active,
            QObject* parent = 0);

    /**
     * Destructor
     */
    ~qdatacube_t();

    /**
     * restrict to this subset
     */
    void restrict(QList<int> set);

    /**
     * Return the number of headers in the given direction
     */
    int headerCount(Qt::Orientation orientation) const;

    /**
     * Return the number of data rows
     */
    int rowCount() const;

    /**
     * Return the number of data columns
     */
    int columnCount() const;

    /**
     * Name of header
     * @param orientation
     * @param index 0 is first header, 1 is next and so on, up until headerCount(orientation)
     */
    QList<QPair<QString,int> > headers(Qt::Orientation orientation, int index) const;

    /**
     * @returns The number of containers for the given row, column
     */
    int cellCount(int row, int column) const;

    /**
     * @returns The actual indices for the given row, column
     */
    QList<int> cellrows(int row, int column) const;

    /**
     * @returns the rows toplevel header
     */
    qdatacube_colrow_t& toplevel_row_header() {
      return *m_rows;
    }

    /**
     * @returns the columns
     */
    qdatacube_colrow_t& toplevel_column_header() {
      return *m_columns;
    }

    /**
     * @return depth for orientation
     */
    int depth(Qt::Orientation orientation);
  public slots:
    /**
     * remove this container from set
     */
    void remove(int container_index);

    /**
     * readds this container to set
     */
    void readd(int container_index);
  signals:
    void changed();
    void begin_remove_row(int);
    void begin_remove_column(int);
    void remove_row(int);
    void remove_column(int);
    void data_changed(int row,int column);

  private:
    std::auto_ptr<qdatacube_colrow_t> m_columns;
    std::auto_ptr<qdatacube_colrow_t> m_rows;
};
}

#endif // QDATACUBE_QDATACUBE_H
