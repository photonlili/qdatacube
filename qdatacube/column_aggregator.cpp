/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "column_aggregator.h"
#include <QStringList>
#include <QAbstractItemModel>
#include <QSet>

namespace qdatacube {

class column_aggregator_t::secret_t {
  public:
    secret_t(int section) : categories(), section(section), trim_right(false), max_chars(3) {
    }
    QStringList categories;
    typedef QHash<QString, int> cat_map_t;
    cat_map_t cat_map;
    int section;
    bool trim_right;
    int max_chars;
};

void column_aggregator_t::set_trim_new_categories_from_right(int max_chars) {
  d->trim_right = true;
  d->max_chars = max_chars;
}

const QList< QString >& column_aggregator_t::categories() {
  Q_ASSERT(d->section < m_underlying_model->columnCount());
  return d->categories;
}

column_aggregator_t::column_aggregator_t(QAbstractItemModel* model, int section): abstract_aggregator_t(model), d(new secret_t(section)) {
  QSet<QString> categories;
  for (int i=0, iend = m_underlying_model->rowCount(); i<iend; ++i) {
    QString cat = m_underlying_model->data(m_underlying_model->index(i, d->section)).toString();
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
  connect(m_underlying_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(refresh_categories_in_rect(QModelIndex,QModelIndex)));
  connect(m_underlying_model, SIGNAL(rowsInserted(const QModelIndex&,int, int)), SLOT(add_rows_to_categories(const QModelIndex&,int,int)));
}

int column_aggregator_t::operator()(int row) {
  Q_ASSERT(m_underlying_model->rowCount() > row);
  QString data = m_underlying_model->data(m_underlying_model->index(row, d->section)).toString();
  if (d->trim_right) {
    data = data.right(d->max_chars);
  }
  int rv = d->cat_map.value(data, 0);
  Q_ASSERT(d->cat_map.contains(data));
  return rv;
}

column_aggregator_t::~column_aggregator_t() {

}

int column_aggregator_t::section() const {
  return d->section;
}

QString column_aggregator_t::name() const
{
  return m_underlying_model->headerData(d->section, Qt::Horizontal).toString();
}

void column_aggregator_t::add_rows_to_categories(const QModelIndex& parent, int start, int end) {
  if (parent.isValid()) {
    return;
  }
  for (int row=start; row<=end; ++row) {
    QString data = m_underlying_model->index(row, d->section).data().toString();
    add_new_category(data);
  }
}

void column_aggregator_t::refresh_categories_in_rect(QModelIndex top_left, QModelIndex bottom_right) {
  if (top_left.parent().isValid()) {
    return;
  }
  if (top_left.column() > d->section || bottom_right.column() < d->section) {
    return;
  }
  for (int row=top_left.row(); row<=bottom_right.row(); ++row) {
    QString data = m_underlying_model->index(row, d->section).data().toString();
    add_new_category(data);
  }

}

void column_aggregator_t::add_new_category(QString data)
{
  int index = d->cat_map.size();
  if (!d->cat_map.contains(data)) {
    for (secret_t::cat_map_t::iterator it = d->cat_map.begin(), iend = d->cat_map.end(); it != iend; ++it) {
      if (it.key() > data) {
        index = qMin(index,it.value());
        ++it.value();
      }
    }
    d->cat_map.insert(data, index);
    d->categories.insert(index, data);
    emit category_added(index);
  }
}

}

#include "column_aggregator.moc"
