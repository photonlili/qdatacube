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
    Q_ASSERT(d->m_filterComponents.length() > 0);
    bool rv = d->m_filterComponents.at(0)->operator()(row);
    for(int i = 1; i < d->m_filterComponents.length(); ++i) {
        rv |= d->m_filterComponents.at(i)->operator()(row);
    }
    return rv;
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
