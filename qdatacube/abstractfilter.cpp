#include "abstractfilter.h"

namespace qdatacube {

class AbstractFilterPrivate {
    public:
        AbstractFilterPrivate(QAbstractItemModel* underlyingModel) : m_underlyingModel(underlyingModel) {}
        QAbstractItemModel* m_underlyingModel;
        QString m_name;
        QString m_shortName;
};
AbstractFilter::AbstractFilter(QAbstractItemModel* underlyingModel)
  : d(new AbstractFilterPrivate(underlyingModel))
{
    Q_ASSERT(underlyingModel);
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
    return d->m_underlyingModel;
}

AbstractFilter::~AbstractFilter() {
    //empty
}

}

#include "abstractfilter.moc"
