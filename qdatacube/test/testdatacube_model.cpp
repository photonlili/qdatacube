/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "testdatacube_model.h"

#include <QtTest/QTest>
#include <QFile>
#include <QStandardItemModel>

#include "column_filter.h"
#include "datacube.h"
#include <datacube_model.h>

QTEST_MAIN(testdatacube_model)

testdatacube_model::testdatacube_model(QObject* parent):
    danishnamecube_t(parent),
    m_cube(0L)
{
  QFile data(DATADIR "/plaincubedata.txt");
  data.open(QIODevice::ReadOnly);
  QStandardItemModel* model = new QStandardItemModel(0, N_COLUMNS, this);
  while (!data.atEnd()) {
    QString line = QString::fromLocal8Bit(data.readLine());
    QStringList columns = line.split(' ');
    Q_ASSERT(columns.size() == N_COLUMNS);
    QList<QStandardItem*> cell_items;
    Q_FOREACH(QString cell, columns) {
      cell.remove("\n");
      cell_items << new QStandardItem(cell);
    }
    model->appendRow(cell_items);
  }
  m_underlying_model = model;
  qDebug() << "Read " << m_underlying_model->rowCount() << " rows";
  m_cube = new datacube_t(m_underlying_model, first_name_filter, last_name_filter, this);

}

void testdatacube_model::testplain() {
  datacube_model_t* m = new datacube_model_t(m_cube, this);
  QCOMPARE(m->rowCount(), m_cube->row_count());
  QCOMPARE(m->columnCount(), m_cube->column_count());
  connect_rowcol_changed(m);
  clear_rowcol_changed();

  // Record which columns and rows to keep
  typedef QPair<QString, int> header_pair_t;
  QList<bool> rows_to_keep;
  for(int section =0; section< m_cube->row_count(); ++section) {
    bool keep = false;
    for (int column=0; column<m_cube->column_count(); ++column) {
      Q_FOREACH(int row, m_cube->elements(section,column)) {
        if (m_underlying_model->data(m_underlying_model->index(row, SEX)).toString() ==  "female") {
          keep = true;
          break;
        }
      }
    }
    rows_to_keep << keep;
  }
  QList<bool> columns_to_keep;
  for(int section =0; section< m_cube->column_count(); ++section) {
    bool keep = false;
    for (int datacube_row = 0; datacube_row < m_cube->row_count(); ++datacube_row) {
      Q_FOREACH(int row, m_cube->elements(datacube_row, section)) {
        if (m_underlying_model->data(m_underlying_model->index(row, SEX)).toString() == "female") {
          keep = true;
          break;
        }
      }
    }
    columns_to_keep << keep;
  }
  // Set filter (and thus trigger a lot of signals about remove/add columns)
  m->datacube()->set_global_filter(sex_filter, 0);
  // Verify the right have been removed (and none added)
  QCOMPARE(m_rows_added.size(),0);
  Q_FOREACH(int row, m_rows_removed) {
    QVERIFY(row>=0);
    QVERIFY(row<rows_to_keep.size());
    QCOMPARE(rows_to_keep.at(row), false);
    rows_to_keep.removeAt(row);
  }
  QVERIFY(!rows_to_keep.contains(false)); // Check that the rows left are the ones to keep, only

  // Verify the right columns have been removed (and none added)
  QCOMPARE(m_columns_added.size(),0);
  Q_FOREACH(int column, m_columns_removed) {
    QVERIFY(column>=0);
    QVERIFY(column<columns_to_keep.size());
    QCOMPARE(columns_to_keep.at(column), false);
    columns_to_keep.removeAt(column);
  }
  QVERIFY(!columns_to_keep.contains(false)); // Check that the columns left are the ones to keep, only

  // Ok, replace filter to get only those with lastname "Jensen"
  clear_rowcol_changed();
  int jensen_cat = last_name_filter->categories(m_underlying_model).indexOf("Jensen");
  QVERIFY(jensen_cat>=0);
  qDebug() << "============= REPLACE WITH JENSEN ===============";
  m->datacube()->set_global_filter(last_name_filter, jensen_cat);
  QCOMPARE(m_columns_added.size(), 0); // Jensen column was not removed above
  QCOMPARE(m_columns_removed.size(), 11); // 12 columns total, but the Thomsen was removed above due to no female Thomsens
  QCOMPARE(m_rows_added.size(), 3); // 3 male first_names were removed above, and readded here: Bertrad, Einar, Karl
  QCOMPARE(m_rows_removed.size(), 5); // 5 female first names does not have a Jensen: Ansersine, Andrea, Lisbeth, Lulu and Tanja
  qDebug() << "============= Reset to no filter ===============";
  clear_rowcol_changed();
  m->datacube()->reset_global_filter();
  QCOMPARE(m_columns_removed.size(),0);
  QCOMPARE(m_rows_removed.size(),0);
  QCOMPARE(m_columns_added.size(),12); // All but the famous Jensens
  QCOMPARE(m_rows_added.size(),8); // All those with no Jensens

}

void testdatacube_model::testdatachange() {
  datacube_model_t* m = new datacube_model_t(m_cube, this);
  QCOMPARE(m->rowCount(), m_cube->row_count());
  QCOMPARE(m->columnCount(), m_cube->column_count());
  connect_rowcol_changed(m);
  clear_rowcol_changed();

}

void testdatacube_model::connect_rowcol_changed(QAbstractItemModel* m) {
  connect(m, SIGNAL(columnsAboutToBeRemoved ( const QModelIndex & , int , int )),
          SLOT(columnsAboutToBeRemoved(QModelIndex,int,int)));
  connect(m, SIGNAL(columnsRemoved ( const QModelIndex & , int , int )),
          SLOT(columnsRemoved(QModelIndex,int,int)));
  connect(m, SIGNAL(columnsAboutToBeInserted( const QModelIndex & , int , int )),
          SLOT(columnsAboutToBeAdded(QModelIndex,int,int)));
  connect(m, SIGNAL(columnsInserted( const QModelIndex & , int , int )),
          SLOT(columnsAdded(QModelIndex,int,int)));
  connect(m, SIGNAL(rowsAboutToBeRemoved ( const QModelIndex & , int , int )),
          SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
  connect(m, SIGNAL(rowsRemoved ( const QModelIndex & , int , int )),
          SLOT(rowsRemoved(QModelIndex,int,int)));
  connect(m, SIGNAL(rowsAboutToBeInserted( const QModelIndex & , int , int )),
          SLOT(rowsAboutToBeAdded(QModelIndex,int,int)));
  connect(m, SIGNAL(rowsInserted( const QModelIndex & , int , int )),
          SLOT(rowsAdded(QModelIndex,int,int)));

}

void testdatacube_model::clear_rowcol_changed() {
  m_rows_added.clear();
  m_columns_added.clear();
  m_rows_removed.clear();
  m_columns_removed.clear();
}

void testdatacube_model::columnsAboutToBeRemoved(const QModelIndex& /*parent*/, int start, int end) {
  for (int i=start; i<=end; ++i) {
    m_columns_removed << i;
  }
  qDebug() << __func__<< start << end << m_columns_removed;
}

void testdatacube_model::columnsAboutToBeAdded(const QModelIndex& /*parent*/, int start, int end) {
  for (int i=start; i<=end; ++i) {
    m_columns_added << i;
  }
  qDebug() << __func__<< start << end << m_columns_added;
}
void testdatacube_model::columnsAdded(const QModelIndex& /*parent*/, int start, int end) {
  qDebug() << __func__<< start << end << m_columns_added;
  for (int i=start; i<=end; ++i) {
    QCOMPARE(m_columns_added.at(m_columns_added.size()-1-end+i), i);
  }

}
void testdatacube_model::columnsRemoved(const QModelIndex& /*parent*/, int start, int end) {
  qDebug() << __func__<< start << end << m_columns_removed;
  for (int i=start; i<=end; ++i) {
    QCOMPARE(m_columns_removed.at(m_columns_removed.size()-1-end+i), i);
  }

}

void testdatacube_model::rowsAboutToBeRemoved(const QModelIndex& /*parent*/, int start, int end) {
  for (int i=start; i<=end; ++i) {
    m_rows_removed << i;
  }
  qDebug() << __func__<< start << end << m_rows_removed;
}

void testdatacube_model::rowsAboutToBeAdded(const QModelIndex& /*parent*/, int start, int end) {
  for (int i=start; i<=end; ++i) {
    m_rows_added << i;
  }
  qDebug() << __func__<< start << end << m_rows_added;
}
void testdatacube_model::rowsAdded(const QModelIndex& /*parent*/, int start, int end) {
  qDebug() << __func__<< start << end << m_rows_added;
  for (int i=start; i<=end; ++i) {
    QCOMPARE(m_rows_added.at(m_rows_added.size()-1-end+i), i);
  }

}
void testdatacube_model::rowsRemoved(const QModelIndex& /*parent*/, int start, int end) {
  qDebug() << __func__<< start << end << m_rows_removed;
  for (int i=start; i<=end; ++i) {
    QCOMPARE(m_rows_removed.at(m_rows_removed.size()-1-end+i), i);
  }

}

#include "testdatacube_model.moc"
