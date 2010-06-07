/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "testplaincube.h"
#include <QStandardItemModel>
#include "datacube.h"
#include "column_filter.h"
#include <datacube_colrow.h>

using namespace qdatacube;

// Note throughout these tests we accesses abstract_filter_t pointers after they have been
// passed to datacube. This is technically not ok, so don't do it.

namespace {
  // Convenient function for printing a datacube to std out
int printdatacube(const datacube_t* datacube) {
  qDebug() << datacube->columnCount() << ", " << datacube->rowCount();
  qDebug() << datacube->headerCount(Qt::Horizontal) << ", " << datacube->headerCount(Qt::Vertical);
  qDebug() << "column headers";
  for (int i=0; i< datacube->headerCount(Qt::Horizontal); ++i) {
    qDebug() << datacube->headers(Qt::Horizontal, i);
  }
  qDebug() << "row headers";
  for (int i=0; i< datacube->headerCount(Qt::Vertical); ++i) {
    qDebug() << datacube->headers(Qt::Vertical, i);
  }
  int total = 0;
  for (int r = 0; r< datacube->rowCount(); ++r) {
    QString row_display;
    QTextStream row_display_stream(&row_display);
    for (int c = 0; c< datacube->columnCount(); ++c) {
      int count = datacube->cellCount(r,c);
      total += count;
      row_display_stream << datacube->cellCount(r,c) << "\t";
    }
    qDebug() << row_display;
  }

  return total;
}
}

void testplaincube::test_basics() {
  QList<int> allactive;
  const int nrows = m_model->rowCount();
  for (int i=0; i<nrows;++i) {
    allactive << i;
  }
  column_filter_t* firstname_filter = new column_filter_t(testplaincube::FIRST_NAME);
  column_filter_t* lastname_filter = new column_filter_t(testplaincube::LAST_NAME);
  datacube_t datacube(m_model, firstname_filter, lastname_filter, allactive);
  int larsen_cat = lastname_filter->categories(m_model).indexOf("Larsen");
  int kim_cat = firstname_filter->categories(m_model).indexOf("Kim");
  QList<int> rows = datacube.cellrows(kim_cat, larsen_cat);
  QCOMPARE(rows.size(), 3);
  Q_FOREACH(int row, rows) {
    QCOMPARE(m_model->data(m_model->index(row, testplaincube::FIRST_NAME)).toString(), QString::fromLocal8Bit("Kim"));
    QCOMPARE(m_model->data(m_model->index(row, testplaincube::LAST_NAME)).toString(), QString::fromLocal8Bit("Larsen"));
  }
  int total = 0;
  for (int row = 0; row < datacube.rowCount(); ++row) {
    for (int column = 0; column < datacube.columnCount(); ++column) {
      total += datacube.cellCount(row, column);
    }
  }
  QCOMPARE(total, m_model->rowCount());
  QCOMPARE(datacube.headerCount(Qt::Vertical), 1);
  QCOMPARE(datacube.headers(Qt::Vertical,0).size(), firstname_filter->categories(m_model).size());
  QCOMPARE(datacube.headerCount(Qt::Horizontal), 1);
  QCOMPARE(datacube.headers(Qt::Horizontal,0).size(), lastname_filter->categories(m_model).size());

}

void testplaincube::test_split() {
  QList<int> allactive;
  const int nrows = m_model->rowCount();
  for (int i=0; i<nrows;++i) {
    allactive << i;
  }
  column_filter_t* kommune_filter = new column_filter_t(testplaincube::KOMMUNE);
  column_filter_t* lastname_filter = new column_filter_t(testplaincube::LAST_NAME);
  datacube_t datacube(m_model, lastname_filter, kommune_filter, allactive);
  column_filter_t* sex_filter = new column_filter_t(testplaincube::SEX);
  datacube.toplevel_column_header().split(sex_filter);
  QCOMPARE(datacube.headerCount(Qt::Horizontal), 2);
  QCOMPARE(datacube.headerCount(Qt::Vertical), 1);
  typedef QPair<QString, int> header_pair_t;
  // Test headers
  int i = 0;
  Q_FOREACH(header_pair_t header, datacube.headers(Qt::Horizontal, 0)) {
    QCOMPARE(header.second,2);
    QCOMPARE(header.first, kommune_filter->categories(m_model).value(i++));
  }

  i = 0;
  Q_FOREACH(header_pair_t header, datacube.headers(Qt::Horizontal, 1)) {
    QCOMPARE(header.second,1);
    QCOMPARE(header.first, sex_filter->categories(m_model).value(i++ % 2));
  }

  //Test sum of data
  int total = 0;
  for (int row = 0; row < datacube.rowCount(); ++row) {
    for (int column = 0; column < datacube.columnCount(); ++column) {
      total += datacube.cellCount(row, column);
    }
  }

  // Pick out a data point: All   male residents in Odense named Pedersen (2)
  int pedersencat = lastname_filter->categories(m_model).indexOf("Pedersen");
  int malecat = sex_filter->categories(m_model).indexOf("male");
  int odensecat = kommune_filter->categories(m_model).indexOf("Odense");
  int col = malecat+sex_filter->categories(m_model).size()*odensecat;
  QCOMPARE(datacube.cellCount(pedersencat, col), 2);
}

void testplaincube::test_columnfilter()
{
  column_filter_t firstname_filter(testplaincube::FIRST_NAME);
  QCOMPARE(firstname_filter.categories(m_model).size(), 14);
  QCOMPARE(firstname_filter.categories(m_model).value(0),QString::fromLocal8Bit("Andersine"));
  QCOMPARE(firstname_filter.categories(m_model).value(7),QString::fromLocal8Bit("Kim"));
  QString baltazar = QString::fromLocal8Bit("Baltazar");
  int cat_of_balt = firstname_filter.categories(m_model).indexOf(baltazar);
  QCOMPARE(cat_of_balt, 2);
  QCOMPARE(firstname_filter(m_model, 0), cat_of_balt);
  QCOMPARE(firstname_filter(m_model, 1), cat_of_balt);
  column_filter_t sex_filter(testplaincube::SEX);
  QCOMPARE(sex_filter.categories(m_model).size(),2);
  QCOMPARE(sex_filter.categories(m_model).value(0), QString::fromLocal8Bit("female"));
  QCOMPARE(sex_filter.categories(m_model).value(1), QString::fromLocal8Bit("male"));

}

testplaincube::testplaincube(QObject* parent): QObject(parent) {
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
  m_model = model;
  qDebug() << "Read " << m_model->rowCount() << " rows";
}

void testplaincube::test_collapse() {
  QList<int> allactive;
  const int nrows = m_model->rowCount();
  for (int i=0; i<nrows;++i) {
    allactive << i;
  }
  column_filter_t* firstname_filter = new column_filter_t(testplaincube::FIRST_NAME);
  column_filter_t* lastname_filter = new column_filter_t(testplaincube::LAST_NAME);
  column_filter_t* sex_filter = new column_filter_t(testplaincube::SEX);
  datacube_t datacube(m_model, firstname_filter, lastname_filter, allactive);
  datacube.toplevel_row_header().split(sex_filter);
  // Splitting first names according to sex should yield no extra rows
  QCOMPARE(datacube.rowCount(), firstname_filter->categories(m_model).size());
  // While splitting last names should. -1 because there are no
  datacube.toplevel_column_header().split(sex_filter);
  // -1 since there are no female Thomsen in our set
  printdatacube(&datacube);
  QCOMPARE(datacube.columnCount(), lastname_filter->categories(m_model).size()*sex_filter->categories(m_model).size() - 1);

}

QTEST_MAIN(testplaincube)

#include "testplaincube.moc"
