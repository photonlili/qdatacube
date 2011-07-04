#ifndef QDATACUBE_COUNT_FORMATTER_H
#define QDATACUBE_COUNT_FORMATTER_H

#include "abstract_formatter.h"
#include "qdatacube_export.h"

namespace qdatacube {

/**
 * Simplest formatter there is: Return the count of each cell/header
 */
class QDATACUBE_EXPORT count_formatter_t : public abstract_formatter_t {
  public:
    count_formatter_t(QAbstractItemModel* underlying_model, qdatacube::datacube_view_t* view);
    virtual QString name() const;
    virtual QString short_name() const;
    virtual QString format(QList< int > rows) const;
};
}
#endif // COUNT_FORMATTER_H
