#include "abstract_filter.h"

namespace qdatacube {


class AbstractFilterPrivate {
    public:
        AbstractFilterPrivate(QAbstractItemModel* underlying_model) : m_underlying_model(underlying_model) {}
        QAbstractItemModel* m_underlying_model;
        QString m_name;
        QString m_shortName;
};
AbstractFilter::AbstractFilter(QAbstractItemModel* underlying_model)
  : d(new AbstractFilterPrivate(underlying_model))
{
  Q_ASSERT(underlying_model);
}

QString AbstractFilter::name() const {
    return d->m_name;
}

void AbstractFilter::setName(const QString& newName) {
    d->m_name = newName;
}

void AbstractFilter::setShortName(const QString& newShortName) {
    d->m_shortName = newShortName;
}

QString AbstractFilter::shortName() const {
    return d->m_shortName;
}

QAbstractItemModel* AbstractFilter::underlyingModel() const {
    return d->m_underlying_model;
}

AbstractFilter::~AbstractFilter() {
    //empty
}







}

#include "abstract_filter.moc"
