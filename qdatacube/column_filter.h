/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef COLUMN_FILTER_H
#define COLUMN_FILTER_H

#include "abstract_filter.h"

#include <QScopedPointer>

#include "qdatacube_export.h"

class QModelIndex;
namespace qdatacube {

class QDATACUBE_EXPORT column_filter_t : public abstract_filter_t {
  Q_OBJECT
  public:
    column_filter_t(int section);
    ~column_filter_t();
    virtual const QList< QString >& categories(const QAbstractItemModel* model);
    virtual int operator()(const QAbstractItemModel* model, int row);
    virtual QString name(const QAbstractItemModel* model) const;
    /**
     * Return section
     */
    int section() const;

    /**
     * trim (new) categories from the right to max max_chars characters
     **/
    void set_trim_new_categories_from_right(int max_chars);
  private:
    class secret_t;
    QScopedPointer<secret_t> d;
};

}
#endif // COLUMN_FILTER_H
