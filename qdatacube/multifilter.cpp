#include "multifilter.h"
#include <QSharedPointer>

namespace qdatacube {
class MultiFilterPrivate {
public:
    MultiFilterPrivate() {};
    QList<AbstractFilter::Ptr> m_filterComponents;
};

MultiFilter::MultiFilter(QAbstractItemModel* underlyingModel): AbstractFilter(underlyingModel), d(new MultiFilterPrivate()) {}

void MultiFilter::addFilter(AbstractFilter::Ptr filter) {
    Q_ASSERT(filter->underlyingModel() == underlyingModel());
    d->m_filterComponents.append(filter);
}

bool MultiFilter::operator()(int row) const {
    Q_FOREACH(AbstractFilter::Ptr filter, d->m_filterComponents) {
       if((*filter)(row))  {
           return true;
       }
    }
    return false;
}

MultiFilter::~MultiFilter() {}

}

#include "multifilter.moc"
