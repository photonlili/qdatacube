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
    secret_t(int section) : categories(), section(section) {}
    QStringList categories;
    QHash<QString, int> cat_map;
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
  }
  return d->categories;
}

column_filter_t::column_filter_t(int section): abstract_filter_t(), d(new secret_t(section)) {

}

int column_filter_t::operator()(const QAbstractItemModel* model, int row) const {
  Q_ASSERT(model);
  Q_ASSERT(model->rowCount() > row);
  QString data = model->data(model->index(row, d->section)).toString();
  int rv = d->cat_map.value(data, -1);
  Q_ASSERT(rv>=0);
  return rv;
}

column_filter_t::~column_filter_t() {

}

}
