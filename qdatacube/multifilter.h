#ifndef MULTI_FILTER_H
#define MULTI_FILTER_H

#include "abstractfilter.h"

template<class T >
class QSharedPointer;
class QAbstractItemModel;

namespace qdatacube {

class MultiFilterPrivate;
/**
 * A filter allowing multiple filters to be combined, by default by OR'ing the
 * result of their operator().
 * Consider extending the functionality with other boolean operators if needed
 */
class QDATACUBE_EXPORT MultiFilter : public AbstractFilter {
    Q_OBJECT
    public:
        typedef QSharedPointer<MultiFilter> Ptr;
        explicit MultiFilter();

        /**
         * @return true if row is to be included
         */
        virtual bool operator()(int row) const;

        /**
         * adds a filter
         * Note: all filters need to share the same underlyingModel
         */
        void addFilter(AbstractFilter* filter);

        /**
         * @return name of filter
         */
        QString name() const ;

        /**
         * @return short name of filter (for corner label)
         */
        QString shortName() const;

        /**
         * @return underlying model
         */
        QAbstractItemModel* underlyingModel() const;
        /**
         * dtor
         */
        virtual ~MultiFilter();
    private:
        QScopedPointer<MultiFilterPrivate> d;

};
}
#endif // MULTI_FILTER_H
