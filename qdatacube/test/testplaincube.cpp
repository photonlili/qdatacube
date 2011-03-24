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
  QList<int> rows = datacube.elements(kim_cat, larsen_cat);
  Q_FOREACH(int row, rows) {
    QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, testplaincube::FIRST_NAME)).toString(), QString::fromLocal8Bit("Kim"));
    QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, testplaincube::LAST_NAME)).toString(), QString::fromLocal8Bit("Larsen"));
  }
  int total = 0;
  for (int row = 0; row < datacube.row_count(); ++row) {
    for (int column = 0; column < datacube.column_count(); ++column) {
      total += datacube.element_count(row, column);
    }
  }
  QCOMPARE(total, m_underlying_model->rowCount());
  QCOMPARE(datacube.header_count(Qt::Vertical), 1);
  QCOMPARE(datacube.headers(Qt::Vertical,0).size(), first_name_filter->categories(m_underlying_model).size());
  QCOMPARE(datacube.header_count(Qt::Horizontal), 1);
  QCOMPARE(datacube.headers(Qt::Horizontal,0).size(), last_name_filter->categories(m_underlying_model).size());

}

void testplaincube::test_split() {
  datacube_t datacube(m_underlying_model, last_name_filter, kommune_filter);
  datacube.split(Qt::Horizontal, 1, sex_filter);
  QCOMPARE(datacube.header_count(Qt::Horizontal), 2);
  QCOMPARE(datacube.header_count(Qt::Vertical), 1);
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
  for (int row = 0; row < datacube.row_count(); ++row) {
    for (int column = 0; column < datacube.column_count(); ++column) {
      total += datacube.element_count(row, column);
    }
  }

  // Pick out a data point: All   male residents in Odense named Pedersen (2)
  int pedersencat = last_name_filter->categories(m_underlying_model).indexOf("Pedersen");
  int malecat = sex_filter->categories(m_underlying_model).indexOf("male");
  int odensecat = kommune_filter->categories(m_underlying_model).indexOf("Odense");
  int col = malecat+sex_filter->categories(m_underlying_model).size()*odensecat;
  QCOMPARE(datacube.element_count(pedersencat, col), 2);

}

void testplaincube::test_collapse1() {
  datacube_t datacube(m_underlying_model, last_name_filter, kommune_filter);
  datacube.split(Qt::Horizontal, 1, sex_filter);
  datacube.collapse(Qt::Horizontal,0);
  QCOMPARE(datacube.header_count(Qt::Horizontal),1);
  QCOMPARE(datacube.header_count(Qt::Vertical),1);
  int total = 0;
  QList<QPair<QString,int> > col_headers = datacube.headers(Qt::Horizontal, 0);
  QList<QPair<QString,int> > row_headers = datacube.headers(Qt::Vertical, 0);
  for (int row = 0; row < datacube.row_count(); ++row) {
    for (int column = 0; column < datacube.column_count(); ++column) {
      Q_FOREACH(int cell, datacube.elements(row, column)) {
        ++total;
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, SEX)).toString(), col_headers.at(column).first);
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, LAST_NAME)).toString(), row_headers.at(row).first);
      }
    }
  }
  QCOMPARE(total, 100);

}

void testplaincube::test_collapse2() {
  datacube_t datacube(m_underlying_model, last_name_filter, kommune_filter);
  datacube.split(Qt::Horizontal, 1, sex_filter);
  datacube.collapse(Qt::Horizontal,1);
  QCOMPARE(datacube.header_count(Qt::Horizontal),1);
  QCOMPARE(datacube.header_count(Qt::Vertical),1);
  int total = 0;
  QList<QPair<QString,int> > col_headers = datacube.headers(Qt::Horizontal, 0);
  QList<QPair<QString,int> > row_headers = datacube.headers(Qt::Vertical, 0);
  for (int row = 0; row < datacube.row_count(); ++row) {
    for (int column = 0; column < datacube.column_count(); ++column) {
      Q_FOREACH(int cell, datacube.elements(row, column)) {
        ++total;
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, KOMMUNE)).toString(), col_headers.at(column).first);
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, LAST_NAME)).toString(), row_headers.at(row).first);
      }
    }
  }
  QCOMPARE(total, 100);

}

void testplaincube::test_collapse3() {
  datacube_t datacube(m_underlying_model, last_name_filter, kommune_filter);
  datacube.split(Qt::Horizontal, 1, age_filter);
  datacube.split(Qt::Horizontal, 2, sex_filter);
  datacube.collapse(Qt::Horizontal,1);
  QCOMPARE(datacube.header_count(Qt::Horizontal),2);
  QCOMPARE(datacube.header_count(Qt::Vertical),1);
  int total = 0;
  QList<QPair<QString,int> > col_headers = datacube.headers(Qt::Horizontal, 1);
  QList<QPair<QString,int> > row_headers = datacube.headers(Qt::Vertical, 0);
  typedef QPair<QString,int> header_pair_t;
  QStringList col0_headers;
  Q_FOREACH(header_pair_t hp, datacube.headers(Qt::Horizontal,0)) {
    for (int i=0; i<hp.second; ++i) {
      col0_headers << hp.first;
    }
  }
  for (int row = 0; row < datacube.row_count(); ++row) {
    for (int column = 0; column < datacube.column_count(); ++column) {
      Q_FOREACH(int cell, datacube.elements(row, column)) {
        ++total;
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, SEX)).toString(), col_headers.at(column).first);
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, KOMMUNE)).toString(), col0_headers.at(column));
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, LAST_NAME)).toString(), row_headers.at(row).first);
      }
    }
  }
  QCOMPARE(total, 100);

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

void testplaincube::test_autocollapse() {
  datacube_t datacube(m_underlying_model, first_name_filter, last_name_filter);
  datacube.split(Qt::Vertical,1, sex_filter);
  // Splitting first names according to sex should yield no extra rows
  QCOMPARE(datacube.row_count(), first_name_filter->categories(m_underlying_model).size());
  // While splitting last names should. -1 because there are no
  datacube.split(Qt::Horizontal, 1, sex_filter);
  // -1 since there are no female Thomsen in our set
  QCOMPARE(datacube.column_count(), last_name_filter->categories(m_underlying_model).size()*sex_filter->categories(m_underlying_model).size() - 1);

}

void testplaincube::test_global_filter() {
  datacube_t datacube(m_underlying_model, first_name_filter, last_name_filter);
  int fourty_cat = age_filter->categories(m_underlying_model).indexOf("40");
  QVERIFY(fourty_cat != -1);

  // Set age filter to include 40-years old only (Expect one result, "Einar Madsen"
  datacube.set_global_filter(age_filter, fourty_cat);
  QCOMPARE(datacube.row_count(),1);
  QCOMPARE(datacube.column_count(),1);
  QList<int> rows = datacube.elements(0,0);
  QCOMPARE(rows.size(), 1);
  int row = rows.front();
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, FIRST_NAME)).toString(), QString::fromLocal8Bit("Einar"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, LAST_NAME)).toString(), QString::fromLocal8Bit("Madsen"));
  // Set age filter to include 41-years old only (Expect one result, "Rigmor Jensen", weighting 76
  int fourtyone_cat = age_filter->categories(m_underlying_model).indexOf("41");
  datacube.set_global_filter(age_filter, fourtyone_cat);
  QCOMPARE(datacube.row_count(),1);
  QCOMPARE(datacube.column_count(),1);
  rows = datacube.elements(0,0);
  QCOMPARE(rows.size(), 1);
  row = rows.front();
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, FIRST_NAME)).toString(), QString::fromLocal8Bit("Rigmor"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, LAST_NAME)).toString(), QString::fromLocal8Bit("Jensen"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, WEIGHT)).toString(), QString::fromLocal8Bit("76"));
  // Get all with that weight (besides Rigmor Jensen, this includes Lulu Petersen)
  int seventysix = weight_filter->categories(m_underlying_model).indexOf("76");
  datacube.set_global_filter(weight_filter, seventysix);
  QCOMPARE(datacube.row_count(),2);
  QCOMPARE(datacube.column_count(),2);
  rows = datacube.elements(1,0);
  QCOMPARE(rows.size(), 1);
  row = rows.front();
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, FIRST_NAME)).toString(), QString::fromLocal8Bit("Rigmor"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, LAST_NAME)).toString(), QString::fromLocal8Bit("Jensen"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, WEIGHT)).toString(), QString::fromLocal8Bit("76"));
  rows = datacube.elements(0,1);
  QCOMPARE(rows.size(), 1);
  row = rows.front();
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, FIRST_NAME)).toString(), QString::fromLocal8Bit("Lulu"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, LAST_NAME)).toString(), QString::fromLocal8Bit("Petersen"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, WEIGHT)).toString(), QString::fromLocal8Bit("76"));

}

void testplaincube::test_deep_header() {
  datacube_t datacube(m_underlying_model, sex_filter, last_name_filter);
  datacube.split(Qt::Horizontal, 1, kommune_filter);
  datacube.split(Qt::Horizontal, 1,sex_filter);
  datacube.split(Qt::Horizontal, 1,age_filter);
  datacube.split(Qt::Horizontal, 1,weight_filter);
  QCOMPARE(datacube.header_count(Qt::Horizontal),5);
  for (int headerno = 0; headerno < datacube.header_count(Qt::Horizontal); ++headerno) {
    int total = 0;
    typedef QPair<QString,int> header_pair_t;
    Q_FOREACH(header_pair_t header_pair, datacube.headers(Qt::Horizontal, headerno)) {
      total += header_pair.second;
    }
    QCOMPARE(total, datacube.column_count());
  }

}

QTEST_MAIN(testplaincube)

#include "testplaincube.moc"
