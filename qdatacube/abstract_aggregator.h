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

class QAbstractItemModel;

namespace qdatacube {

/**
 * aggregate elements into a number of categories
 * Note that category_removed is unsupported as yet.
 */
class QDATACUBE_EXPORT abstract_aggregator_t : public QObject {
  Q_OBJECT
  public:
    explicit abstract_aggregator_t(QAbstractItemModel* model, QObject* parent = 0);

    /**
     * @param row number in m_model
     * @returns the category number for row, 0 <= return value < categories().size()
     *
     */
    virtual int operator()(int row) = 0;

    /**
     * @returns list of categories for this aggregator
     */
    virtual const QList<QString>& categories() = 0;

    /**
     * @returns an name for this aggregator. Default implementation returns "unnamed";
     */
    virtual QString name() const {
      return QString::fromLocal8Bit("unnamed");
    }

    /**
    * @return underlying model
    */
    QAbstractItemModel* underlying_model() const {
      return m_underlying_model;
    }
  Q_SIGNALS:
    /**
     * Implementors must emit this signal when a category has been added
     * @param index index of removed category
     */
    void category_added(int index) const;

    /**
     * Implementors must emit this signal when a category has been removed
     * NOTE: This is unsupported currently.
     * @param index index of new category
     */
    void category_removed(int index) const;
  protected:
    QAbstractItemModel* m_underlying_model;
};

}

#endif // QDATACUBE_ABSTRACT_AGGREGATOR_H
