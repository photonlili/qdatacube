#include "abstract_filter.h"

namespace qdatacube {


class AbstractFilterPrivate {
    public:
        AbstractFilterPrivate(QAbstractItemModel* underlying_model) : m_underlying_model(underlying_model) {}
        QAbstractItemModel* m_underlying_model;
        QString m_name;
        QString m_shortName;
};
abstract_filter_t::abstract_filter_t(QAbstractItemModel* underlying_model, QObject* parent)
  : QObject(parent), d(new AbstractFilterPrivate(underlying_model))
{
  Q_ASSERT(underlying_model);
}

QString abstract_filter_t::name() const {
    return d->m_name;
}

void abstract_filter_t::setName(const QString& newName) {
    d->m_name = newName;
}

void abstract_filter_t::setShortName(const QString& newShortName) {
    d->m_shortName = newShortName;
}

QString abstract_filter_t::short_name() const {
    return d->m_shortName;
}

QAbstractItemModel* abstract_filter_t::underlying_model() const {
    return d->m_underlying_model;
}

abstract_filter_t::~abstract_filter_t() {
    //empty
}







}

#include "abstract_filter.moc"
