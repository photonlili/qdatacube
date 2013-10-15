/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef COLUMN_FILTER_H
#define COLUMN_FILTER_H

#include "abstract_aggregator.h"

#include <QScopedPointer>

#include "qdatacube_export.h"
#include <QAbstractItemModel>

class QModelIndex;
namespace qdatacube {

class ColumnAggregatorPrivate;
class QDATACUBE_EXPORT column_aggregator_t : public abstract_aggregator_t {
  Q_OBJECT
  public:
    column_aggregator_t(QAbstractItemModel* model,  int section);
    ~column_aggregator_t();
    virtual int operator()(int row) const;
    /**
     * Return section
     */
    int section() const;

    virtual int categoryCount() const;

    virtual QVariant categoryHeaderData(int category, int role = Qt::DisplayRole) const;

    /**
     * trim (new) categories from the right to max max_chars characters
     **/
    void set_trim_new_categories_from_right(int max_chars);
  public Q_SLOTS:
    /**
     * Recalculate categories. This is also triggered automatically when the number of changed or removed rows
     * exceeds the half the current size
     */
    void reset_categories();
  private:
    QScopedPointer<ColumnAggregatorPrivate> d;
    void add_new_category(QString data);
    void remove_category(QString category);
  private Q_SLOTS:
    void refresh_categories_in_rect(QModelIndex top_left, QModelIndex bottom_right);
    void add_rows_to_categories(const QModelIndex& parent, int start, int end);
    void remove_rows_from_categories(const QModelIndex& parent, int start, int end);
};

}
#endif // COLUMN_FILTER_H
