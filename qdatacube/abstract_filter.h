#ifndef ABSTRACT_FILTER_H
#define ABSTRACT_FILTER_H

#include <QObject>
#include "qdatacube_export.h"

class QAbstractItemModel;

namespace qdatacube {

class QDATACUBE_EXPORT abstract_filter_t : public QObject {
  Q_OBJECT
  public:
    explicit abstract_filter_t(QAbstractItemModel* underlying_model, QObject* parent = 0);

    /**
     * @return true if row is to be included
     */
    virtual bool operator()(int row) = 0;

    /**
     * @return name of filter
     */
    virtual QString name() const = 0;

    /**
     * @return underlying model
     */
    QAbstractItemModel* underlying_model() const {
      return m_underlying_model;
    }
  protected:
    QAbstractItemModel* m_underlying_model;

};
}
#endif // ABSTRACT_FILTER_H
