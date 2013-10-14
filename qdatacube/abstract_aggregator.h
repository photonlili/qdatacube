/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef QDATACUBE_ABSTRACT_AGGREGATOR_H
#define QDATACUBE_ABSTRACT_AGGREGATOR_H
#include <QList>

#include "qdatacube_export.h"
#include <QString>
#include <QObject>
#include <QVariant>

class QAbstractItemModel;

namespace qdatacube {

/**
 * aggregate elements into a number of categories
 */
class AbstractAggregatorPrivate;
class QDATACUBE_EXPORT abstract_aggregator_t : public QObject {
  Q_OBJECT
  public:
    explicit abstract_aggregator_t(QAbstractItemModel* model);

    /**
     * @param row number in m_model
     * @returns the category number for row, 0 <= return value < categories().size()
     *
     */
    virtual int operator()(int row) = 0;

    /**
     * @return the number of categories in this aggregator
     */
    virtual int categoryCount() const = 0;

    /**
     * @param  category to query
     * @param role header data role to query
     * @return the headerdata for a category and a role.
     */
    virtual QVariant categoryHeaderData(int category, int role = Qt::DisplayRole) const = 0;

    /**
     * @returns an name for this aggregator. Default implementation returns "unnamed";
     */
    virtual QString name() const;

    /**
    * @return underlying model
    */
    QAbstractItemModel* underlying_model() const;

    /**
     * dtor
     */
    virtual ~abstract_aggregator_t();
  Q_SIGNALS:
    /**
     * Implementors must emit this signal when a category has been added
     * @param index index of removed category
     */
    void category_added(int index) const;

    /**
     * Implementors must emit this signal when a category has been removed
     * @param index index of new category
     */
    void category_removed(int index) const;
  private:
      QScopedPointer<AbstractAggregatorPrivate> d;
};

}

#endif // QDATACUBE_ABSTRACT_AGGREGATOR_H
