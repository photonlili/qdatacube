#ifndef QDATACUBE_DATACUBE_P_H
#define QDATACUBE_DATACUBE_P_H

#include <QObject>
#include <tr1/memory>

#include "cell.h"
#include "datacube.h"
class QAbstractItemModel;

namespace qdatacube {

struct CellPoint {
    long row;
    long column;
    CellPoint(int row, int column) : row(row), column(column) {}
};

class DatacubePrivate : public QObject {
    Q_OBJECT
    public:
        DatacubePrivate(Datacube* datacube, const QAbstractItemModel* model,
                std::tr1::shared_ptr<AbstractAggregator> row_aggregator,
                std::tr1::shared_ptr<AbstractAggregator> column_aggregator);
        DatacubePrivate(Datacube* datacube, const QAbstractItemModel* model);
        Datacube* q;
        int compute_row_section_for_index(int index) {
            return compute_section_for_index(Qt::Vertical, index);
        }
        int compute_column_section_for_index(int index) {
            return compute_section_for_index(Qt::Horizontal, index);
        }
        int compute_section_for_index(Qt::Orientation orientation, int index);
        const QList<int>& cell(long int bucket_row, long int bucket_column) const;
        int hasCell(long int bucket_row, long int bucket_column) const;
        void setCell(long int bucket_row, long int bucket_column, QList< int > cell_content);
        void setCell(CellPoint point, QList<int> cell_content);
        void cellAppend(long int bucket_row, long int bucket_column, int to_add);
        void cellAppend(CellPoint point, QList<int> listadd);
        int bucket_to_row(int bucket_row) const;
        int bucket_to_column(int bucket_column) const;
        /**
        * Renumber cells from start by adding adjustment
        */
        void renumber_cells(int start, int adjustment);
        bool cellRemoveOne(long int row, long int column, int index);

        const QAbstractItemModel* model;
        Datacube::Aggregators row_aggregators;
        Datacube::Aggregators col_aggregators;
        QVector<unsigned> row_counts;
        QVector<unsigned> col_counts;
        typedef QHash<long, QList<int> > cells_t;
        Datacube::GlobalFilters global_filters;
        typedef QHash<int, Cell> reverse_index_t;
        reverse_index_t reverse_index;
        QList<datacube_selection_t*> selection_models;
        const QList<int> empty_list;
        cells_t cells;

        void remove(int index);
        void add(int index);
        void split_row(int headerno, std::tr1::shared_ptr< AbstractAggregator > aggregator);
        void split_column(int headerno, std::tr1::shared_ptr< AbstractAggregator > aggregator);
        void aggregator_category_added(std::tr1::shared_ptr< qdatacube::AbstractAggregator > aggregator, int headerno, int index, Qt::Orientation orientation);
        void aggregator_category_removed(std::tr1::shared_ptr< qdatacube::AbstractAggregator > aggregator, int headerno, int index, Qt::Orientation orientation);

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
        void bucket_for_element(int element, qdatacube::Cell& result) const;

        /**
        * Add a selection model for bucket change notification
        */
        void add_selection_model(datacube_selection_t* selection);

        /**
        * @returns true if included by the current set of global filters
        */
        bool filtered_in(int element) const;
    public Q_SLOTS:
        void update_data(QModelIndex topleft, QModelIndex bottomRight);
        void remove_data(QModelIndex parent, int start, int end);
        void insert_data(QModelIndex parent, int start, int end);
        void slot_columns_changed(int column, int count);
        void slot_rows_changed(int row, int count);
        void slot_aggregator_category_added(int index);
        void slot_aggregator_category_removed(int);
        void remove_selection_model(QObject* selection_model);
};

}

#endif // QDATACUBE_DATACUBE_P_H
