/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "column_filter.h"
#include <QStringList>
#include <QAbstractItemModel>
#include <QSet>

namespace qdatacube {

class column_filter_t::secret_t {
  public:
    secret_t(int section) : categories(), section(section) {
    }
    QStringList categories;
    typedef QHash<QString, int> cat_map_t;
    cat_map_t cat_map;
    int section;
};

const QList< QString >& column_filter_t::categories(const QAbstractItemModel* model) const {
  Q_ASSERT(model);
  Q_ASSERT(d->section < model->columnCount());
  if (d->categories.isEmpty()) {
    QSet<QString> categories;
    for (int i=0, iend = model->rowCount(); i<iend; ++i) {
      categories << model->data(model->index(i, d->section)).toString();
    }
    d->categories = categories.toList();
    qSort(d->categories);
    for(int i=0; i<d->categories.size(); ++i) {
      QString cat = d->categories.at(i);
      d->cat_map.insert(cat, i);
    }
    if (d->categories.isEmpty()) {
      d->categories << model->tr("other"); // Have to have at least one
    }
  }
  return d->categories;
}

column_filter_t::column_filter_t(int section): abstract_filter_t(), d(new secret_t(section)) {
}

int column_filter_t::operator()(const QAbstractItemModel* model, int row) const {
  Q_ASSERT(model);
  Q_ASSERT(model->rowCount() > row);
  if (d->categories.isEmpty()) {
    // Create categories
    categories(model);
  }
  QString data = model->data(model->index(row, d->section)).toString();
  int rv = d->cat_map.value(data, -1);
  if (rv == -1) {
    QStringList::iterator lb = qLowerBound(d->categories.begin(), d->categories.end(), data);
    int index = lb - d->categories.begin();
    for (secret_t::cat_map_t::iterator it = d->cat_map.begin(), iend = d->cat_map.end(); it != iend; ++it) {
      if (*it>=index) {
        ++(*it);
      }
    }
    d->categories.insert(lb, data);
    d->cat_map.insert(data, index);
    emit category_added(index);
    rv = index;
  }
  return rv;
}

column_filter_t::~column_filter_t() {

}

int column_filter_t::section() const {
  return d->section;
}

QString column_filter_t::name(const QAbstractItemModel* model) const
{
  return model->headerData(d->section, Qt::Horizontal).toString();
}


}

#include "column_filter.moc"
