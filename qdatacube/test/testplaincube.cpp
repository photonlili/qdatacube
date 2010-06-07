/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "testplaincube.h"
#include <QStandardItemModel>
#include "datacube.h"
#include "column_filter.h"

using namespace qdatacube;

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
      cell_items << new QStandardItem(cell);
    }
    model->appendRow(cell_items);
  }
  m_model = model;
  qDebug() << "Read " << m_model->rowCount() << " rows";
}

QTEST_MAIN(testplaincube)

#include "testplaincube.moc"
