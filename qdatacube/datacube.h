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
#include <QAbstractItemModel>

class QAbstractItemModel;

namespace qdatacube {

class abstract_filter_t;

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
            QObject* parent = 0);

    /**
     * Destructor
     */
    ~datacube_t();

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
     * @return depth (number of headers/filters) for orientation
     */
    int depth(Qt::Orientation orientation);

    /**
     * Set global filter. Containers in category are included, the rest is rejected.
     */
    void set_global_filter(std::tr1::shared_ptr<abstract_filter_t> filter, int category);

    /**
     * Set global filter. Convenience overload. Filter is claimed by this datacube and will be deleted at some point
     */
    void set_global_filter(abstract_filter_t* filter, int category);

    /**
     * Remove global filter.
     */
    void reset_global_filter();

    /**
     * Split header with filter.
     */
    void split(Qt::Orientation orientation, int headerno, std::tr1::shared_ptr<abstract_filter_t> filter);

    /**
     * Split header with filter.
     * ownership of filter is filter is claimed and the filter is deleted at some point.
     */
    void split(Qt::Orientation orientation, int headerno, abstract_filter_t* filter);

  signals:
    /**
     * rows are about to be removed
     */
    void rows_about_to_be_removed(int index, int count);

    /**
     * columns are about to be removed
     */
    void columns_about_to_be_removed(int index, int count);

    /**
     * rows have been removed
     */
    void rows_removed(int index, int count);

    /**
     * columns have been removed
     */
    void columns_removed(int index, int count);

    /**
     * rows are about to be removed
     */
    void rows_about_to_be_added(int index, int count);

    /**
     * columns are about to be added
     */
    void columns_about_to_be_added(int index, int count);

    /**
     * rows have been added
     */
    void rows_added(int index, int count);

    /**
     * columns have been added
     */
    void columns_added(int index, int count);

    /**
     * header data has changed
     * data_changed will still be emitted
     */
    void headers_changed(Qt::Orientation, int first, int last);


    /**
     * The value in cell has changed
     */
    void data_changed(int row,int column);

  private Q_SLOTS:
    void update_data(QModelIndex topleft, QModelIndex bottomRight);
    void remove_data(QModelIndex parent, int start, int end);
    void insert_data(QModelIndex parent, int start, int end);
    void slot_columns_changed(int column, int count);
    void slot_rows_changed(int row, int count);

  private:
    void remove(int index);
    void add(int index);

    class secret_t;
    QScopedPointer<secret_t> d;
};
}

#endif // Q_DATACUBE_DATACUBE_H
