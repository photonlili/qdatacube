/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef Q_DATACUBE_DATACUBE_H
#define Q_DATACUBE_DATACUBE_H

#include "qdatacube_export.h"

#include <QObject>
#include <QPair>
#include <tr1/memory>

class QAbstractItemModel;

namespace qdatacube {

class abstract_filter_t;
class datacube_colrow_t;

class QDATACUBE_EXPORT datacube_t : public QObject {
  Q_OBJECT
  public:
    /**
     * Construct simple 2-dimensional datacube with the 2 filters
     * @param row_filter initial filter. Ownership is claimed, and filter will be deleted
     * @param column_filter initial filter. Ownership is claimed, and filter will be deleted
     * @param active limited to these containers
     */
    datacube_t(const QAbstractItemModel* model,
            abstract_filter_t* row_filter,
            abstract_filter_t* column_filter,
            const QList<int>& active,
            QObject* parent = 0);

    /**
     * Construct simple 2-dimensional datacube with the 2 filters
     * @param row_filter initial filter.
     * @param column_filter initial filter.
     * @param active limited to these containers
     */
    datacube_t(const QAbstractItemModel* model,
            std::tr1::shared_ptr<abstract_filter_t> row_filter,
            std::tr1::shared_ptr<abstract_filter_t> column_filter,
            const QList<int>& active,
            QObject* parent = 0);

    /**
     * Destructor
     */
    ~datacube_t();

    /**
     * restrict to this subset
     * TODO: Remove, see ticket #115
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
     * @return name of header, number of columns spanned
     * @param orientation
     * @param index 0 is first header, 1 is next and so on, up until headerCount(orientation)
     */
    QList<QPair<QString,int> > headers(Qt::Orientation orientation, int index) const;

    /**
     * @returns The number of elements for the given row, column
     */
    int cellCount(int row, int column) const;

    /**
     * @returns The actual indices for the given row, column
     */
    QList<int> cellrows(int row, int column) const;

    /**
     * @returns the rows toplevel header
     */
    datacube_colrow_t& toplevel_row_header() ;

    /**
     * @returns the columns
     */
    datacube_colrow_t& toplevel_column_header() ;

    /**
     * @return depth (number of headers/filters) for orientation
     */
    int depth(Qt::Orientation orientation);
  public slots:
    /**
     * remove this container from set
     * TODO: Should be superflous with ticket #115
     */
    void remove(int container_index);

    /**
     * readds this container to set
     * TODO: Should be superflous with ticket #115
     */
    void readd(int container_index);
  signals:
    /**
     * Datacube has changed in some way.
     * TODO: Should be superflous with ticket #115
     */
    void changed();

    /**
     * A row is about to be removed
     */
    void row_about_to_be_removed(int index);

    /**
     * A row is about to be removed
     */
    void column_about_to_be_removed(int);

    /**
     * A row has been removed
     */
    void row_removed(int);

    /**
     * A column has been removed
     */
    void column_removed(int);

    /**
     * The value in cell has changed
     */
    void data_changed(int row,int column);

  private:
    class secret_t;
    QScopedPointer<secret_t> d;
};
}

#endif // Q_DATACUBE_DATACUBE_H
