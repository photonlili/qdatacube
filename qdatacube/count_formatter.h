#ifndef QDATACUBE_COUNT_FORMATTER_H
#define QDATACUBE_COUNT_FORMATTER_H

#include "abstract_formatter.h"
#include "qdatacube_export.h"

namespace qdatacube {

/**
 * Simplest formatter there is: Return the count of each cell/header
 */
class QDATACUBE_EXPORT CountFormatter : public AbstractFormatter {
    public:
        CountFormatter(QAbstractItemModel* underlying_model, qdatacube::DatacubeView* view = 0L);
        virtual QString format(QList< int > rows) const;
    protected:
        virtual void update(qdatacube::AbstractFormatter::UpdateType updateType);
};
}
#endif // COUNT_FORMATTER_H
