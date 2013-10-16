#ifndef ABSTRACT_FILTER_H
#define ABSTRACT_FILTER_H

#include <QObject>
#include "qdatacube_export.h"

class QAbstractItemModel;

namespace qdatacube {

class AbstractFilterPrivate;
class QDATACUBE_EXPORT abstract_filter_t : public QObject {
  Q_OBJECT
  public:
    explicit abstract_filter_t(QAbstractItemModel* underlying_model);

    /**
     * @return true if row is to be included
     */
    virtual bool operator()(int row) const = 0;

    /**
     * @return name of filter
     */
    QString name() const ;

    /**
     * @return short name of filter (for corner label)
     */
    QString short_name() const;

    /**
     * @return underlying model
     */
    QAbstractItemModel* underlying_model() const;
    /**
     * dtor
     */
    virtual ~abstract_filter_t();
    protected:
        /**
         * sets name of this filter to \param newName
         */
        void setName(const QString& newName);
        /**
         * sets short name for this filter to \param newShortName
         * should in general be kept at at most 3 chars.
         */
        void setShortName(const QString& newShortName);
    private:
        QScopedPointer<AbstractFilterPrivate> d;

};
}
#endif // ABSTRACT_FILTER_H
