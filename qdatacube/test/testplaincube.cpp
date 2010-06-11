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

void testplaincube::test_basics() {
  datacube_t datacube(m_underlying_model, first_name_filter, last_name_filter);
  int larsen_cat = last_name_filter->categories(m_underlying_model).indexOf("Larsen");
  int kim_cat = first_name_filter->categories(m_underlying_model).indexOf("Kim");
  QList<int> rows = datacube.cellrows(kim_cat, larsen_cat);
  QCOMPARE(rows.size(), 3);
  Q_FOREACH(int row, rows) {
    QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, testplaincube::FIRST_NAME)).toString(), QString::fromLocal8Bit("Kim"));
    QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, testplaincube::LAST_NAME)).toString(), QString::fromLocal8Bit("Larsen"));
  }
  int total = 0;
  for (int row = 0; row < datacube.rowCount(); ++row) {
    for (int column = 0; column < datacube.columnCount(); ++column) {
      total += datacube.cellCount(row, column);
    }
  }
  QCOMPARE(total, m_underlying_model->rowCount());
  QCOMPARE(datacube.headerCount(Qt::Vertical), 1);
  QCOMPARE(datacube.headers(Qt::Vertical,0).size(), first_name_filter->categories(m_underlying_model).size());
  QCOMPARE(datacube.headerCount(Qt::Horizontal), 1);
  QCOMPARE(datacube.headers(Qt::Horizontal,0).size(), last_name_filter->categories(m_underlying_model).size());

}

void testplaincube::test_split() {
  datacube_t datacube(m_underlying_model, last_name_filter, kommune_filter);
  datacube.toplevel_column_header().split(sex_filter);
  QCOMPARE(datacube.headerCount(Qt::Horizontal), 2);
  QCOMPARE(datacube.headerCount(Qt::Vertical), 1);
  typedef QPair<QString, int> header_pair_t;
  // Test headers
  int i = 0;
  Q_FOREACH(header_pair_t header, datacube.headers(Qt::Horizontal, 0)) {
    QCOMPARE(header.second,2);
    QCOMPARE(header.first, kommune_filter->categories(m_underlying_model).value(i++));
  }

  i = 0;
  Q_FOREACH(header_pair_t header, datacube.headers(Qt::Horizontal, 1)) {
    QCOMPARE(header.second,1);
    QCOMPARE(header.first, sex_filter->categories(m_underlying_model).value(i++ % 2));
  }

  //Test sum of data
  int total = 0;
  for (int row = 0; row < datacube.rowCount(); ++row) {
    for (int column = 0; column < datacube.columnCount(); ++column) {
      total += datacube.cellCount(row, column);
    }
  }

  // Pick out a data point: All   male residents in Odense named Pedersen (2)
  int pedersencat = last_name_filter->categories(m_underlying_model).indexOf("Pedersen");
  int malecat = sex_filter->categories(m_underlying_model).indexOf("male");
  int odensecat = kommune_filter->categories(m_underlying_model).indexOf("Odense");
  int col = malecat+sex_filter->categories(m_underlying_model).size()*odensecat;
  QCOMPARE(datacube.cellCount(pedersencat, col), 2);
}

void testplaincube::test_columnfilter()
{
  column_filter_t first_name_filter(testplaincube::FIRST_NAME);
  QCOMPARE(first_name_filter.categories(m_underlying_model).size(), 14);
  QCOMPARE(first_name_filter.categories(m_underlying_model).value(0),QString::fromLocal8Bit("Andersine"));
  QCOMPARE(first_name_filter.categories(m_underlying_model).value(7),QString::fromLocal8Bit("Kim"));
  QString baltazar = QString::fromLocal8Bit("Baltazar");
  int cat_of_balt = first_name_filter.categories(m_underlying_model).indexOf(baltazar);
  QCOMPARE(cat_of_balt, 2);
  QCOMPARE(first_name_filter(m_underlying_model, 0), cat_of_balt);
  QCOMPARE(first_name_filter(m_underlying_model, 1), cat_of_balt);
  column_filter_t sex_filter(testplaincube::SEX);
  QCOMPARE(sex_filter.categories(m_underlying_model).size(),2);
  QCOMPARE(sex_filter.categories(m_underlying_model).value(0), QString::fromLocal8Bit("female"));
  QCOMPARE(sex_filter.categories(m_underlying_model).value(1), QString::fromLocal8Bit("male"));

}

testplaincube::testplaincube(QObject* parent): danishnamecube_t(parent) {
  load_model_data("plaincubedata.txt");
}

void testplaincube::test_collapse() {
  datacube_t datacube(m_underlying_model, first_name_filter, last_name_filter);
  datacube.toplevel_row_header().split(sex_filter);
  // Splitting first names according to sex should yield no extra rows
  QCOMPARE(datacube.rowCount(), first_name_filter->categories(m_underlying_model).size());
  // While splitting last names should. -1 because there are no
  datacube.toplevel_column_header().split(sex_filter);
  // -1 since there are no female Thomsen in our set
  QCOMPARE(datacube.columnCount(), last_name_filter->categories(m_underlying_model).size()*sex_filter->categories(m_underlying_model).size() - 1);

}

void testplaincube::test_global_filter() {
  datacube_t datacube(m_underlying_model, first_name_filter, last_name_filter);
  int fourty_cat = age_filter->categories(m_underlying_model).indexOf("40");
  QVERIFY(fourty_cat != -1);

  // Set age filter to include 40-years old only (Expect one result, "Einar Madsen"
  datacube.set_global_filter(age_filter, fourty_cat);
  QCOMPARE(datacube.rowCount(),1);
  QCOMPARE(datacube.columnCount(),1);
  QList<int> rows = datacube.cellrows(0,0);
  QCOMPARE(rows.size(), 1);
  int row = rows.front();
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, FIRST_NAME)).toString(), QString::fromLocal8Bit("Einar"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, LAST_NAME)).toString(), QString::fromLocal8Bit("Madsen"));
  // Set age filter to include 41-years old only (Expect one result, "Rigmor Jensen", weighting 76
  int fourtyone_cat = age_filter->categories(m_underlying_model).indexOf("41");
  datacube.set_global_filter(age_filter, fourtyone_cat);
  QCOMPARE(datacube.rowCount(),1);
  QCOMPARE(datacube.columnCount(),1);
  rows = datacube.cellrows(0,0);
  QCOMPARE(rows.size(), 1);
  row = rows.front();
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, FIRST_NAME)).toString(), QString::fromLocal8Bit("Rigmor"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, LAST_NAME)).toString(), QString::fromLocal8Bit("Jensen"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, WEIGHT)).toString(), QString::fromLocal8Bit("76"));
  // Get all with that weight (besides Rigmor Jensen, this includes Lulu Petersen)
  int seventysix = weight_filter->categories(m_underlying_model).indexOf("76");
  datacube.set_global_filter(weight_filter, seventysix);
  QCOMPARE(datacube.rowCount(),2);
  QCOMPARE(datacube.columnCount(),2);
  rows = datacube.cellrows(1,0);
  QCOMPARE(rows.size(), 1);
  row = rows.front();
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, FIRST_NAME)).toString(), QString::fromLocal8Bit("Rigmor"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, LAST_NAME)).toString(), QString::fromLocal8Bit("Jensen"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, WEIGHT)).toString(), QString::fromLocal8Bit("76"));
  rows = datacube.cellrows(0,1);
  QCOMPARE(rows.size(), 1);
  row = rows.front();
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, FIRST_NAME)).toString(), QString::fromLocal8Bit("Lulu"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, LAST_NAME)).toString(), QString::fromLocal8Bit("Petersen"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, WEIGHT)).toString(), QString::fromLocal8Bit("76"));

}

void testplaincube::test_deep_header() {
  datacube_t datacube(m_underlying_model, sex_filter, last_name_filter);
  datacube.toplevel_column_header().split(kommune_filter);
  datacube.toplevel_column_header().split(sex_filter);
  datacube.toplevel_column_header().split(age_filter);
  datacube.toplevel_column_header().split(weight_filter);
  QCOMPARE(datacube.headerCount(Qt::Horizontal),5);
  for (int headerno = 0; headerno < datacube.headerCount(Qt::Horizontal); ++headerno) {
    int total = 0;
    typedef QPair<QString,int> header_pair_t;
    Q_FOREACH(header_pair_t header_pair, datacube.headers(Qt::Horizontal, headerno)) {
      total += header_pair.second;
    }
    QCOMPARE(total, datacube.columnCount());
  }

}

QTEST_MAIN(testplaincube)

#include "testplaincube.moc"
