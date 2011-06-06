/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef QDATACUBE_ABSTRACT_FILTER_H
#define QDATACUBE_ABSTRACT_FILTER_H
#include <QList>

#include "qdatacube_export.h"
#include <QString>
#include <QObject>

class QAbstractItemModel;

namespace qdatacube {

/**
 * container filter for recaps interface
 * Note that these filters are currently designed to be independent of the underlying model
 * Note that category_removed is unsupported as yet.
 */
class QDATACUBE_EXPORT abstract_filter_t : public QObject {
  Q_OBJECT
  public:
    /**
     * @param model
     * @param row number in model
     * @returns the category number for container, 0 <= return value < categories().size()
     *
     */
    virtual int operator()(const QAbstractItemModel* model, int row) = 0;

    /**
     * @returns list of categories for this filter
     */
    virtual const QList<QString>& categories(const QAbstractItemModel* model) = 0;

    /**
     * @returns an name for this filter. Default implementation returns "unnamed";
     */
    virtual QString name(const QAbstractItemModel* model) const {
      Q_UNUSED(model);
      return QString::fromLocal8Bit("unnamed");
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
};

}

#endif // QDATACUBE_ABSTRACT_FILTER_H
