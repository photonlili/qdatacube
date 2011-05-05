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

class datacube_colrow_t;


class abstract_filter_t;

/**
 * row: row in datacube
 * column: column in datacube
 * section: row or column in datacube
 * element: row number in underlying model
 * bucket(no): "raw" section, that is, including autocollapsed (empty) rows and columns
 */
class QDATACUBE_EXPORT datacube_t : public QObject {
  Q_OBJECT
  public:
    /**
     * Construct simple 2-dimensional datacube with the 2 filters
     * @param underlying_model the model whose rows are the data elements in the datacube
     * @param row_filter initial filter. Ownership is claimed, and filter will be deleted
     * @param column_filter initial filter. Ownership is claimed, and filter will be deleted
     */
    datacube_t(const QAbstractItemModel* underlying_model,
               abstract_filter_t* row_filter,
               abstract_filter_t* column_filter,
               QObject* parent = 0);

    /**
     * Construct simple 2-dimensional datacube with the 2 filters
     * Construct simple 2-dimensional datacube with the 2 filters
     * @param underlying_model the model whose rows are the data elements in the datacube
     * @param row_filter initial filter.
     * @param column_filter initial filter.
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
     * Return the number of headers in the given direction. The sum of the
     * headers in both directions would give the total dimension of the (hyper)cube.
     * In other words, this is the number of dimensions projected to the orientation.
     */
    int header_count(Qt::Orientation orientation) const;

    /**
     * Return the number of rows in the cube
     */
    int row_count() const;

    /**
     * Return the number of columns in the cube
     */
    int column_count() const;

    /**
     * @return pair of (name of header, number of columns spanned)
     * @param orientation
     * @param index 0 is first header, 1 is next and so on, up until headerCount(orientation)
     */
    QList<QPair<QString,int> > headers(Qt::Orientation orientation, int index) const;

    /**
     * @returns The number of elements for the given row, column.
     * equivalent to elements(row,column).size()
     */
    int element_count(int row, int column) const;

    /**
     * @returns The elements in the given row, column
     */
    QList<int> elements(int row, int column) const;

    /**
     * Set global filter. Elements in "category" are included, the rest are excluded
     */
    void set_global_filter(std::tr1::shared_ptr<abstract_filter_t> filter, int category);

    /**
     * Set global filter. Convenience overload. Filter is claimed by this datacube and will be deleted at some point
     * equivalent to set_global_filter(std::tr1::shared_ptr<abstract_filter_t*>(filter), category);
     */
    void set_global_filter(abstract_filter_t* filter, int category);

    /**
     * Remove global filter.
     */
    void reset_global_filter();

    /**
     * Split header with filter.
     * @param orientation split by column or row
     * @param headerno which header to split. 0 means this is the new topmost split,
     *                 header_count(orientation) is the bottommost.
     * @param filter filter to use. Each non-empty category will give a new row or column
     */
    void split(Qt::Orientation orientation, int headerno, std::tr1::shared_ptr<abstract_filter_t> filter);

    /**
     * Split header with filter.
     * convenience overload, same as split(orientation, headerno, std::tr1::shared_ptr<abstract_filter_t>(filter));
     */
    void split(Qt::Orientation orientation, int headerno, abstract_filter_t* filter);

    /**
     * Collapse header, removing it from datacube. Requries headercount(orientation)>=2
     * @param orientation collapse row or column
     * @param headerno which header to remove. Must be less that header_count(orientation)
     */
    void collapse(Qt::Orientation orientation, int headerno);

    /**
     * @returns the section for
     * @param orientation
     * and
     * @param element
     */
    int section_for_element(int element, Qt::Orientation orientation) const;

    /**
     * @return the current section for the element (which might be wrong if you are listening to e.g. rows_about_to_be_removed)
     * TODO: Find a better name for this.
     */
    int section_for_element_internal(int element, Qt::Orientation orientation) const;

    /**
     * @return pointer to global filter
     */
    std::tr1::shared_ptr<abstract_filter_t> global_filter() const;

    /**
     * @return the underlying model
     */
    const QAbstractItemModel* underlying_model() const;

    /**
     * Debug function: Dump internal state
     */
    void dump(bool cells, bool rowcounts, bool col_counts) const;

    /**
     * Run some internal checks. Useful for debugging
     */
    void check();

  Q_SIGNALS:
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
    void rows_about_to_be_inserted(int index, int count);

    /**
     * columns are about to be added
     */
    void columns_about_to_be_inserted(int index, int count);

    /**
     * rows have been added
     */
    void rows_inserted(int index, int count);

    /**
     * columns have been added
     */
    void columns_inserted(int index, int count);

    /**
     * header data has changed
     * data_changed will still be emitted
     */
    void headers_changed(Qt::Orientation, int first, int last);

    /**
     * The value in cell has changed
     */
    void data_changed(int row,int column);

    /**
     * Datacube is about to be completely changed. All cell and headers can change
     */
    void about_to_be_reset();

    /**
     * Datacube has completely changed. All cell and headers can change
     */
    void reset();

  private Q_SLOTS:
    void update_data(QModelIndex topleft, QModelIndex bottomRight);
    void remove_data(QModelIndex parent, int start, int end);
    void insert_data(QModelIndex parent, int start, int end);
    void slot_columns_changed(int column, int count);
    void slot_rows_changed(int row, int count);
    void slot_filter_category_added(int index);
    void slot_filter_category_removed(int);

  private:
    void remove(int index);
    void add(int index);
    void split_row(int headerno, std::tr1::shared_ptr< abstract_filter_t > filter);
    void split_column(int headerno, std::tr1::shared_ptr< abstract_filter_t > filter);
    void filter_category_added(std::tr1::shared_ptr< qdatacube::abstract_filter_t > filter, int headerno, int index, Qt::Orientation orientation);

    class secret_t;
    QScopedPointer<secret_t> d;

};
}

#endif // Q_DATACUBE_DATACUBE_H
