#include "multifilter.h"

namespace qdatacube {
class MultiFilterPrivate {
public:
    MultiFilterPrivate() {};
    QList<AbstractFilter*> m_filterComponents;
};

MultiFilter::MultiFilter(): AbstractFilter(0L), d(new MultiFilterPrivate()) {}

void MultiFilter::addFilter(AbstractFilter* filter) {
    if(underlyingModel()) {
        Q_ASSERT(filter->underlyingModel() == underlyingModel());
    }
    d->m_filterComponents.append(filter);
}

bool MultiFilter::operator()(int row) const {
    Q_FOREACH(AbstractFilter* filter, d->m_filterComponents) {
       if((*filter)(row))  {
           return true;
       }
    }
    return false;
}

QAbstractItemModel* MultiFilter::underlyingModel() const {
    if(d->m_filterComponents.isEmpty()) {
        return 0L;
    } else {
        return d->m_filterComponents.at(0)->underlyingModel();
    }
}


MultiFilter::~MultiFilter() {}

}

#include "multifilter.moc"
