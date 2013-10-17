/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "column_aggregator.h"
#include <QStringList>
#include <QAbstractItemModel>
#include <QSet>

namespace qdatacube {

class ColumnAggregatorPrivate {
  public:
    ColumnAggregatorPrivate(int section) : categories(), section(section), trim_right(false), max_chars(3), possible_removed_counts(0) {
    }
    QStringList categories;
    typedef QHash<QString, int> cat_map_t;
    cat_map_t cat_map;
    int section;
    bool trim_right;
    int max_chars;
    int possible_removed_counts;
};

void ColumnAggregator::setTrimNewCategoriesFromRight(int max_chars) {
  d->trim_right = true;
  d->max_chars = max_chars;
  // Rebuild categories
  QStringList cats;
  ColumnAggregatorPrivate::cat_map_t map;
  Q_FOREACH(QString cat, d->categories) {
    cat = cat.right(max_chars);
    if (cats.isEmpty() || cats.last() != cat) {
      cats << cat;
    }
  }
  d->categories = cats;
  for (int i=0; i<cats.size(); ++i) {
    map.insert(cats.at(i),i);
  }
  d->cat_map = map;
}

int ColumnAggregator::categoryCount() const {
    Q_ASSERT(d->section < underlyingModel()->columnCount());
    return d->categories.size();
}

QVariant ColumnAggregator::categoryHeaderData(int category, int role) const {
    if(role == Qt::DisplayRole) {
        return d->categories.at(category);
    }
    return QVariant();
}

ColumnAggregator::ColumnAggregator(QAbstractItemModel* model, int section): AbstractAggregator(model), d(new ColumnAggregatorPrivate(section)) {
  QSet<QString> categories;
  for (int i=0, iend = underlyingModel()->rowCount(); i<iend; ++i) {
    QString cat = underlyingModel()->data(underlyingModel()->index(i, d->section)).toString();
    if (d->trim_right) {
      cat = cat.right(d->max_chars);
    }
    categories << cat;
  }
  d->categories = categories.toList();
  qSort(d->categories);
  for(int i=0; i<d->categories.size(); ++i) {
    QString cat = d->categories.at(i);
    d->cat_map.insert(cat, i);
  }
  connect(underlyingModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(refresh_categories_in_rect(QModelIndex,QModelIndex)));
  connect(underlyingModel(), SIGNAL(rowsInserted(const QModelIndex&,int, int)), SLOT(add_rows_to_categories(const QModelIndex&,int,int)));
  connect(underlyingModel(), SIGNAL(modelReset()), SLOT(resetCategories()));
  connect(underlyingModel(), SIGNAL(rowsRemoved(const QModelIndex&,int, int)), SLOT(remove_rows_from_categories(const QModelIndex&,int,int)));

  setName(underlyingModel()->headerData(d->section, Qt::Horizontal).toString());
}

int ColumnAggregator::operator()(int row) const {
  Q_ASSERT(underlyingModel()->rowCount() > row);
  QString data = underlyingModel()->data(underlyingModel()->index(row, d->section)).toString();
  if (d->trim_right) {
    data = data.right(d->max_chars);
  }
  int rv = d->cat_map.value(data, 0);
  Q_ASSERT(d->cat_map.contains(data));
  return rv;
}

ColumnAggregator::~ColumnAggregator() {

}

int ColumnAggregator::section() const {
  return d->section;
}

void ColumnAggregator::add_rows_to_categories(const QModelIndex& parent, int start, int end) {
  if (parent.isValid()) {
    return;
  }
  for (int row=start; row<=end; ++row) {
    QString data = underlyingModel()->index(row, d->section).data().toString();
    if (d->trim_right) {
      data = data.right(d->max_chars);
    }
    add_new_category(data);
  }
}

void ColumnAggregator::refresh_categories_in_rect(QModelIndex top_left, QModelIndex bottom_right) {
  if (top_left.parent().isValid()) {
    return;
  }
  if (top_left.column() > d->section || bottom_right.column() < d->section) {
    return;
  }
  for (int row=top_left.row(); row<=bottom_right.row(); ++row) {
    QString data = underlyingModel()->index(row, d->section).data().toString();
    if (d->trim_right) {
      data = data.right(d->max_chars);
    }
    add_new_category(data);
  }
  d->possible_removed_counts += bottom_right.row() - top_left.row();
  if (d->possible_removed_counts*2 > underlyingModel()->rowCount()) {
    resetCategories();
  }

}

void ColumnAggregator::add_new_category(QString data)
{
  int index = d->cat_map.size();
  if (!d->cat_map.contains(data)) {
    for (ColumnAggregatorPrivate::cat_map_t::iterator it = d->cat_map.begin(), iend = d->cat_map.end(); it != iend; ++it) {
      if (it.key() > data) {
        index = qMin(index,it.value());
        ++it.value();
      }
    }
    d->cat_map.insert(data, index);
    d->categories.insert(index, data);
    emit categoryAdded(index);
  }
}
void ColumnAggregator::remove_category(QString category)
{
  const int index = d->cat_map.value(category, -1);
  if (index<0) {
    Q_ASSERT(false);
    return;
  }
  for (int i=index+1; i<d->categories.size(); ++i) {
    int rv = --d->cat_map[d->categories[i]];
    Q_ASSERT(rv>=index);
    Q_UNUSED(rv);
  }
  d->categories.removeAt(index);
  emit categoryRemoved(index);

}


void ColumnAggregator::remove_rows_from_categories(const QModelIndex& parent, int start, int end) {
  if (parent.isValid()) {
    return;
  }
  d->possible_removed_counts += (end-start);
  if (d->possible_removed_counts*2 > underlyingModel()->rowCount()) {
    resetCategories();
  }
}

void ColumnAggregator::resetCategories() {
  QSet<QString> categories;
  for (int i=0, iend = underlyingModel()->rowCount(); i<iend; ++i) {
    QString cat = underlyingModel()->data(underlyingModel()->index(i, d->section)).toString();
    if (d->trim_right) {
      cat = cat.right(d->max_chars);
    }
    categories << cat;
  }
  Q_FOREACH(QString cat, d->categories) {
    if (!categories.contains(cat)) {
      remove_category(cat);
    }
    categories.remove(cat);
  }
  Q_FOREACH(QString cat, categories) {
    add_new_category(cat);
  }

}

}

#include "column_aggregator.moc"
