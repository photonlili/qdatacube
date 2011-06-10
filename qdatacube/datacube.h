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


class abstract_aggregator_t;
class cell_t;
class datacube_selection_t;
class datacube_colrow_t;

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
     * Construct simple 2-dimensional datacube with the 2 aggregators
     * @param underlying_model the model whose rows are the data elements in the datacube
     * @param row_aggregator initial aggregator. Ownership is claimed, and aggregator will be deleted
     * @param column_aggregator initial aggregator. Ownership is claimed, and aggregator will be deleted
     */
    datacube_t(const QAbstractItemModel* underlying_model,
               abstract_aggregator_t* row_aggregator,
               abstract_aggregator_t* column_aggregator,
               QObject* parent = 0);

    /**
     * Construct simple 2-dimensional datacube with the 2 aggregators
     * @param underlying_model the model whose rows are the data elements in the datacube
     * @param row_aggregator initial aggregator.
     * @param column_aggregator initial aggregator.
     */
    datacube_t(const QAbstractItemModel* model,
            std::tr1::shared_ptr<abstract_aggregator_t> row_aggregator,
            std::tr1::shared_ptr<abstract_aggregator_t> column_aggregator,
            QObject* parent = 0);

    /**
     * Construct datacube with no aggregators at all.
     */
    explicit datacube_t(const QAbstractItemModel* model, QObject* parent = 0);

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
     * @returns the category index
     */
    int category_index(Qt::Orientation orientation, int header_index, int section) const;

    /**
     * Add global filter.
     */
    void add_global_filter(std::tr1::shared_ptr<abstract_filter_t> filter);

    /**
     * Add global filter. Convenience overload. Filter is claimed by this datacube and will be deleted at some point
     * equivalent to add_global_filter(std::tr1::shared_ptr<abstract_filter_t*>(filter));
     */
    void add_global_filter(abstract_filter_t* filter);

    /**
     * Remove all global filters
     */
    void reset_global_filter();

    /**
     * Remove global filter from list
     */
    void remove_global_filter(std::tr1::shared_ptr<abstract_filter_t> filter);

    /**
     * Remove global filter from list
     */
    bool remove_global_filter(qdatacube::abstract_filter_t* filter);

    /**
     * Split header with aggregator.
     * @param orientation split by column or row
     * @param headerno which header to split. 0 means this is the new topmost split,
     *                 header_count(orientation) is the bottommost.
     * @param aggregator aggregator to use. Each non-empty category will give a new row or column
     */
    void split(Qt::Orientation orientation, int headerno, std::tr1::shared_ptr<abstract_aggregator_t> aggregator);

    /**
     * Split header with aggregator.
     * convenience overload, same as split(orientation, headerno, std::tr1::shared_ptr<abstract_aggregator_t>(aggregator));
     */
    void split(Qt::Orientation orientation, int headerno, abstract_aggregator_t* aggregator);

    /**
     * Collapse header, removing it from datacube. Requries headercount(orientation)>=2
     * @param orientation collapse row or column
     * @param headerno which header to remove. Must be less that header_count(orientation)
     */
    void collapse(Qt::Orientation orientation, int headerno);

    /**
     * @returns the section (i.e, row for Qt::Vertical and column for Qt::Horizontal) for
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

    typedef QList<std::tr1::shared_ptr<abstract_filter_t> > global_filters_t;
    typedef QList<std::tr1::shared_ptr<abstract_aggregator_t> > aggregators_t;

    /**
     * @return Return all global filters in effect with their categories
     */
    QList< std::tr1::shared_ptr< abstract_filter_t > > global_filters() const;

    /**
     * @return list of column aggregators, in order
     */
    aggregators_t column_aggregators() const;

    /**
     * @return list of row aggregators, in order
     */
    aggregators_t row_aggregators() const;

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
    void slot_aggregator_category_added(int index);
    void slot_aggregator_category_removed(int);
    void remove_selection_model(QObject* selection_model);

  private:
    void remove(int index);
    void add(int index);
    void split_row(int headerno, std::tr1::shared_ptr< abstract_aggregator_t > aggregator);
    void split_column(int headerno, std::tr1::shared_ptr< abstract_aggregator_t > aggregator);
    void aggregator_category_added(std::tr1::shared_ptr< qdatacube::abstract_aggregator_t > aggregator, int headerno, int index, Qt::Orientation orientation);

    /**
     * @returns the number of buckets (i.e. sections including empty sections) in datacube for
     * @param orientation
     */
    int number_of_buckets(Qt::Orientation orientation) const;

    /**
     * @return elements for bucket row, bucket column
     */
    QList<int> elements_in_bucket(int row, int column) const;

    /**
     * @return the bucket for row
     */
    int bucket_for_row(int row) const;

    /**
    * @return the bucket for column
    */
    int bucket_for_column(int column) const;

    /**
     * @return section for bucket row
     */
    int section_for_bucket_row(int bucket_row) const;

    /**
     * @return section for bucket row
     */
    int section_for_bucket_column(int bucket_column) const;

    /**
     * find cell_t with bucket for element
     * @param element element to look for
     * @param result the cell with the element, or an invalid cell
     * The strange interface is to avoid exporting cell_t
     */
    void bucket_for_element(int element, qdatacube::cell_t& result) const;

    /**
     * Add a selection model for bucket change notification
     */
    void add_selection_model(datacube_selection_t* selection);

    /**
     * @returns true if included by the current set of global filters
     */
    bool filtered_in(int element) const;

    class secret_t;
    QScopedPointer<secret_t> d;

    friend class datacube_selection_t;

};
}

#endif // Q_DATACUBE_DATACUBE_H
