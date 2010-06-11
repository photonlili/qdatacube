/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef QDATACUBE_ABSTRACT_FILTER_H
#define QDATACUBE_ABSTRACT_FILTER_H
#include <QList>

#include "qdatacube_export.h"
#include <QString>

class QAbstractItemModel;

namespace qdatacube {

/**
 * container filter for recaps interface
 */
class QDATACUBE_EXPORT abstract_filter_t {
  public:
    /**
     * @param model
     * @param row number in model
     * @returns the category number for container, 0 <= return value < categories().size()
     *
     */
    virtual int operator()(const QAbstractItemModel* model, int row) const = 0;

    /**
     * @returns list of categories for this filter
     */
    virtual const QList<QString>& categories(const QAbstractItemModel* model) const = 0;

    /**
     * @returns an name for this filter. Default implementation returns "unnamed";
     */
    virtual QString name(const QAbstractItemModel* model) const {
      Q_UNUSED(model);
      return QString::fromLocal8Bit("unnamed");
    }
};

}
#endif // QDATACUBE_ABSTRACT_FILTER_H
