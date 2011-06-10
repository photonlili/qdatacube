/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "testplaincube.h"
#include <QStandardItemModel>
#include "datacube.h"
#include "column_aggregator.h"

using namespace qdatacube;

void testplaincube::test_empty_cube() {
  datacube_t datacube(m_underlying_model);
  QCOMPARE(datacube.header_count(Qt::Horizontal),0);
  QCOMPARE(datacube.header_count(Qt::Vertical),0);
  QCOMPARE(datacube.row_count(),1);
  QCOMPARE(datacube.column_count(),1);
  QCOMPARE(datacube.element_count(0,0), m_underlying_model->rowCount());

  // Split and collapse it in both directions, then both at once
  datacube.split(Qt::Horizontal, 0, sex_aggregator);
  datacube.collapse(Qt::Horizontal,0);
  datacube.split(Qt::Vertical, 0, sex_aggregator);
  datacube.collapse(Qt::Vertical,0);
  datacube.split(Qt::Horizontal, 0, kommune_aggregator);
  datacube.split(Qt::Vertical, 0, sex_aggregator);
  datacube.collapse(Qt::Horizontal,0);
  datacube.collapse(Qt::Vertical,0);

  // Check for survival
  QCOMPARE(datacube.header_count(Qt::Horizontal),0);
  QCOMPARE(datacube.header_count(Qt::Vertical),0);
  QCOMPARE(datacube.row_count(),1);
  QCOMPARE(datacube.column_count(),1);
  QCOMPARE(datacube.element_count(0,0), m_underlying_model->rowCount());


}

void testplaincube::test_basics() {
  datacube_t datacube(m_underlying_model, first_name_aggregator, last_name_aggregator);
  int larsen_cat = last_name_aggregator->categories().indexOf("Larsen");
  int kim_cat = first_name_aggregator->categories().indexOf("Kim");
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
  QCOMPARE(datacube.headers(Qt::Vertical,0).size(), first_name_aggregator->categories().size());
  QCOMPARE(datacube.header_count(Qt::Horizontal), 1);
  QCOMPARE(datacube.headers(Qt::Horizontal,0).size(), last_name_aggregator->categories().size());

}

void testplaincube::test_split() {
  datacube_t datacube(m_underlying_model, last_name_aggregator, kommune_aggregator);
  datacube.split(Qt::Horizontal, 1, sex_aggregator);
  QCOMPARE(datacube.header_count(Qt::Horizontal), 2);
  QCOMPARE(datacube.header_count(Qt::Vertical), 1);
  typedef QPair<QString, int> header_pair_t;
  // Test headers
  int i = 0;
  Q_FOREACH(header_pair_t header, datacube.headers(Qt::Horizontal, 0)) {
    QCOMPARE(header.second,2);
    QCOMPARE(header.first, kommune_aggregator->categories().value(i++));
  }

  i = 0;
  Q_FOREACH(header_pair_t header, datacube.headers(Qt::Horizontal, 1)) {
    QCOMPARE(header.second,1);
    QCOMPARE(header.first, sex_aggregator->categories().value(i++ % 2));
  }

  //Test sum of data
  int total = 0;
  for (int row = 0; row < datacube.row_count(); ++row) {
    for (int column = 0; column < datacube.column_count(); ++column) {
      total += datacube.element_count(row, column);
    }
  }

  // Pick out a data point: All   male residents in Odense named Pedersen (2)
  int pedersencat = last_name_aggregator->categories().indexOf("Pedersen");
  int malecat = sex_aggregator->categories().indexOf("male");
  int odensecat = kommune_aggregator->categories().indexOf("Odense");
  int col = malecat+ sex_aggregator->categories().size()*odensecat;
  QCOMPARE(datacube.element_count(pedersencat, col), 2);

}

void testplaincube::dotest_splittwice(Qt::Orientation direction)
{
  // direction and parallel: direction to test
  // normal: the other direction
  std::tr1::shared_ptr<abstract_aggregator_t> row_aggregator;
  std::tr1::shared_ptr<abstract_aggregator_t> column_aggregator;
  Qt::Orientation normal;
  if (direction == Qt::Horizontal) {
    column_aggregator = kommune_aggregator;
    row_aggregator = last_name_aggregator;
    normal = Qt::Vertical;
  } else {
    row_aggregator = kommune_aggregator;
    column_aggregator = last_name_aggregator;
    normal = Qt::Horizontal;
  }
  datacube_t datacube(m_underlying_model, row_aggregator, column_aggregator);
  datacube.split(direction, 1, sex_aggregator);
  datacube.split(direction, 1, age_aggregator);
  QCOMPARE(datacube.header_count(direction),3);
  QCOMPARE(datacube.header_count(normal),1);
  int total = 0;
  QList<QPair<QString,int> > normal_headers = datacube.headers(normal, 0);
  QCOMPARE(normal_headers.size(), last_name_aggregator->categories().size());
  QList<QPair<QString,int> > parallel_headers0 = datacube.headers(direction, 0);
  const int nkommuner = kommune_aggregator->categories().size();
  QCOMPARE(parallel_headers0.size(), nkommuner);
  QList<QPair<QString,int> > parellel_headers1 = datacube.headers(direction, 1);
  QList<QPair<QString,int> > parellel_headers2 = datacube.headers(direction, 2);
  typedef QPair<QString,int> header_pair_t;
  QStringList parallel0_headers;
  Q_FOREACH(header_pair_t hp, datacube.headers(direction,0)) {
    for (int i=0; i<hp.second; ++i) {
      parallel0_headers << hp.first;
    }
  }
  QStringList parellel1_headers;
  Q_FOREACH(header_pair_t hp, datacube.headers(direction,1)) {
    for (int i=0; i<hp.second; ++i) {
      parellel1_headers << hp.first;
    }
  }
  QStringList parellel2_headers;
  Q_FOREACH(header_pair_t hp, datacube.headers(direction,2)) {
    for (int i=0; i<hp.second; ++i) {
      parellel2_headers << hp.first;
    }
  }
  const int parellel_count = (direction == Qt::Horizontal) ? datacube.column_count() : datacube.row_count();
  const int normal_count = (direction == Qt::Vertical) ? datacube.column_count() : datacube.row_count();
  QCOMPARE(parellel2_headers.size(),parellel_count);
  for (int n = 0; n < normal_count; ++n) {
    for (int p = 0; p < parellel_count; ++p) {
      QList<int> elements = (direction == Qt::Horizontal) ? datacube.elements(n,p) : datacube.elements(p,n);
      Q_FOREACH(int cell, elements) {
        ++total;
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, LAST_NAME)).toString(), normal_headers.at(n).first);
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, KOMMUNE)).toString(), parallel0_headers.at(p));
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, AGE)).toString(), parellel1_headers.at(p));
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, SEX)).toString(), parellel2_headers.at(p));
      }
    }
  }
  QCOMPARE(total, 100);

}

void testplaincube::test_split_twice_horizontal() {
  dotest_splittwice(Qt::Horizontal);
}

void testplaincube::test_split_twice_vertical()
{
  dotest_splittwice(Qt::Vertical);
}

void testplaincube::test_collapse1() {
  datacube_t datacube(m_underlying_model, last_name_aggregator, kommune_aggregator);
  datacube.split(Qt::Horizontal, 1, sex_aggregator);
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
  datacube_t datacube(m_underlying_model, last_name_aggregator, kommune_aggregator);
  datacube.split(Qt::Horizontal, 1, sex_aggregator);
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
  do_testcollapse3(Qt::Horizontal);
}

void testplaincube::test_collapse3_vertical() {
  do_testcollapse3(Qt::Vertical);

}

void testplaincube::do_testcollapse3(Qt::Orientation orientation) {
  const bool horizontal = (orientation == Qt::Horizontal);
  const Qt::Orientation normal = horizontal ? Qt::Vertical : Qt::Horizontal;
  datacube_t datacube(m_underlying_model, horizontal ? last_name_aggregator : kommune_aggregator, horizontal ? kommune_aggregator: last_name_aggregator);
  datacube.split(orientation, 1, age_aggregator);
  datacube.split(orientation, 2, sex_aggregator);
  datacube.collapse(orientation,1);
  QCOMPARE(datacube.header_count(orientation),2);
  QCOMPARE(datacube.header_count(normal),1);
  int total = 0;
  QList<QPair<QString,int> > parallel_headers = datacube.headers(orientation, 1);
  QList<QPair<QString,int> > normal_headers = datacube.headers(normal, 0);
  typedef QPair<QString,int> header_pair_t;
  QStringList parallel_0_headers;
  Q_FOREACH(header_pair_t hp, datacube.headers(orientation,0)) {
    for (int i=0; i<hp.second; ++i) {
      parallel_0_headers << hp.first;
    }
  }
  QCOMPARE(datacube.headers(orientation, 0).size(), kommune_aggregator->categories().size());
  for (int row = 0; row < datacube.row_count(); ++row) {
    for (int column = 0; column < datacube.column_count(); ++column) {
      Q_FOREACH(int cell, datacube.elements(row, column)) {
        ++total;
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, SEX)).toString(), parallel_headers.at(horizontal ? column: row).first);
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, KOMMUNE)).toString(), parallel_0_headers.at(horizontal ? column: row));
        QCOMPARE(m_underlying_model->data(m_underlying_model->index(cell, LAST_NAME)).toString(), normal_headers.at(horizontal ? row : column).first);
      }
    }
  }
  QCOMPARE(total, 100);

}


void testplaincube::test_columnaggregator()
{
  column_aggregator_t first_name_aggregator(m_underlying_model, testplaincube::FIRST_NAME);
  QCOMPARE(first_name_aggregator.categories().size(), 14);
  QCOMPARE(first_name_aggregator.categories().value(0),QString::fromLocal8Bit("Andersine"));
  QCOMPARE(first_name_aggregator.categories().value(7),QString::fromLocal8Bit("Kim"));
  QString baltazar = QString::fromLocal8Bit("Baltazar");
  int cat_of_balt = first_name_aggregator.categories().indexOf(baltazar);
  QCOMPARE(cat_of_balt, 2);
  QCOMPARE(first_name_aggregator(0), cat_of_balt);
  QCOMPARE(first_name_aggregator(1), cat_of_balt);
  column_aggregator_t  sex_aggregator(m_underlying_model ,testplaincube::SEX);
  QCOMPARE(sex_aggregator.categories().size(),2);
  QCOMPARE(sex_aggregator.categories().value(0), QString::fromLocal8Bit("female"));
  QCOMPARE(sex_aggregator.categories().value(1), QString::fromLocal8Bit("male"));

}

testplaincube::testplaincube(QObject* parent): danishnamecube_t(parent) {
  load_model_data("plaincubedata.txt");
}

void testplaincube::test_autocollapse() {
  datacube_t datacube(m_underlying_model, first_name_aggregator, last_name_aggregator);
  datacube.split(Qt::Vertical,1, sex_aggregator);
  // Splitting first names according to sex should yield no extra rows
  QCOMPARE(datacube.row_count(), first_name_aggregator->categories().size());
  // While splitting last names should. -1 because there are no
  datacube.split(Qt::Horizontal, 1, sex_aggregator);
  // -1 since there are no female Thomsen in our set
  QCOMPARE(datacube.column_count(), last_name_aggregator->categories().size()*sex_aggregator->categories().size() - 1);

}

void testplaincube::test_global_filter() {
  datacube_t datacube(m_underlying_model, first_name_aggregator, last_name_aggregator);
  int fourty_cat = age_aggregator->categories().indexOf("40");
  QVERIFY(fourty_cat != -1);

  // Set age filter to include 40-years old only (Expect one result, "Einar Madsen"
  datacube.add_global_filter(age_aggregator, fourty_cat);
  QCOMPARE(datacube.row_count(),1);
  QCOMPARE(datacube.column_count(),1);
  QList<int> rows = datacube.elements(0,0);
  QCOMPARE(rows.size(), 1);
  int row = rows.front();
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, FIRST_NAME)).toString(), QString::fromLocal8Bit("Einar"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, LAST_NAME)).toString(), QString::fromLocal8Bit("Madsen"));
  // Set age filter to include 41-years old only (Expect one result, "Rigmor Jensen", weighting 76
  int fourtyone_cat = age_aggregator->categories().indexOf("41");
  datacube.reset_global_filter();
  datacube.add_global_filter(age_aggregator, fourtyone_cat);
  QCOMPARE(datacube.row_count(),1);
  QCOMPARE(datacube.column_count(),1);
  rows = datacube.elements(0,0);
  QCOMPARE(rows.size(), 1);
  row = rows.front();
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, FIRST_NAME)).toString(), QString::fromLocal8Bit("Rigmor"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, LAST_NAME)).toString(), QString::fromLocal8Bit("Jensen"));
  QCOMPARE(m_underlying_model->data(m_underlying_model->index(row, WEIGHT)).toString(), QString::fromLocal8Bit("76"));
  // Get all with that weight (besides Rigmor Jensen, this includes Lulu Petersen)
  int seventysix = weight_aggregator->categories().indexOf("76");
  datacube.remove_global_filter(age_aggregator);
  datacube.add_global_filter(weight_aggregator, seventysix);
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
  datacube_t datacube(m_underlying_model, sex_aggregator, last_name_aggregator);
  datacube.split(Qt::Horizontal, 1, kommune_aggregator);
  datacube.split(Qt::Horizontal, 1,sex_aggregator);
  datacube.split(Qt::Horizontal, 1,age_aggregator);
  datacube.split(Qt::Horizontal, 1,weight_aggregator);
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
void testplaincube::test_section_for_element_internal()
{
  datacube_t datacube(m_underlying_model, last_name_aggregator, first_name_aggregator);
  datacube.split(Qt::Vertical, 0, age_aggregator);
  datacube.split(Qt::Horizontal, 0, sex_aggregator);
  datacube.split(Qt::Vertical, 1, kommune_aggregator);
  datacube.split(Qt::Horizontal, 1, kommune_aggregator);
  datacube.collapse(Qt::Vertical, 1);
  datacube.collapse(Qt::Horizontal, 1);
  for (int element=0; element<m_underlying_model->rowCount(); ++element) {
    int row = datacube.section_for_element_internal(element, Qt::Vertical);
    int column = datacube.section_for_element_internal(element, Qt::Horizontal);
    QVERIFY(datacube.elements(row, column).contains(element));
    QCOMPARE(datacube.section_for_element(element, Qt::Vertical), row);
    QCOMPARE(datacube.section_for_element(element, Qt::Horizontal), column);
  }
}

void testplaincube::test_reverse_index()
{
  datacube_t datacube(m_underlying_model, last_name_aggregator, first_name_aggregator);
  datacube.split(Qt::Vertical, 0, age_aggregator);
  datacube.split(Qt::Horizontal, 0, sex_aggregator);
  datacube.split(Qt::Vertical, 1, kommune_aggregator);
  datacube.split(Qt::Horizontal, 1, kommune_aggregator);
  datacube.collapse(Qt::Vertical, 1);
  datacube.collapse(Qt::Horizontal, 1);
  for (int element=0; element<m_underlying_model->rowCount(); ++element) {
    int row = datacube.section_for_element(element, Qt::Vertical);
    int column = datacube.section_for_element(element, Qt::Horizontal);
    QVERIFY(datacube.elements(row, column).contains(element));
  }

}

void testplaincube::test_add_category() {
  QStandardItemModel* tmp_model = copy_model();
  std::tr1::shared_ptr<abstract_aggregator_t> sex_aggregator(new column_aggregator_t(tmp_model, SEX));
  std::tr1::shared_ptr<abstract_aggregator_t> kommune_aggregator(new column_aggregator_t(tmp_model, KOMMUNE));
  std::tr1::shared_ptr<abstract_aggregator_t> first_name_aggregator(new column_aggregator_t(tmp_model, FIRST_NAME));
  std::tr1::shared_ptr<abstract_aggregator_t> last_name_aggregator(new column_aggregator_t(tmp_model, LAST_NAME));
  datacube_t datacube(tmp_model, last_name_aggregator, first_name_aggregator);
  datacube.split(Qt::Horizontal, 0, sex_aggregator);
  datacube.split(Qt::Horizontal, 2, kommune_aggregator);
  QList<QStandardItem*> row;
  for (int c=0; c<tmp_model->columnCount(); ++c) {
    switch (static_cast<columns_t>(c)) {
      case FIRST_NAME:
        row << new QStandardItem("Agnete");
        break;
      case LAST_NAME:
        row << new QStandardItem("Johansen");
        break;
      case SEX:
        row << new QStandardItem("female");
        break;
      case AGE:
        row << new QStandardItem("20");
        break;
      case WEIGHT:
        row << new QStandardItem("80");
        break;
      case KOMMUNE:
        row << new QStandardItem("Hellerup");
        break;
      case N_COLUMNS:
        Q_ASSERT(false);
    }
  }
  tmp_model->appendRow(row);
  QVERIFY(first_name_aggregator->categories().contains("Agnete"));
  QCOMPARE(tmp_model->rowCount(), 101);
  QStringList column0_headers;
  typedef QPair<QString, int> header_pair_t;
  Q_FOREACH(header_pair_t hp, datacube.headers(Qt::Horizontal,0)) {
    for (int i=0; i<hp.second; ++i) {
      column0_headers << hp.first;
    }
  }
  QStringList column1_headers;
  typedef QPair<QString, int> header_pair_t;
  Q_FOREACH(header_pair_t hp, datacube.headers(Qt::Horizontal,1)) {
    for (int i=0; i<hp.second; ++i) {
      column1_headers << hp.first;
    }
  }
  QStringList column2_headers;
  typedef QPair<QString, int> header_pair_t;
  Q_FOREACH(header_pair_t hp, datacube.headers(Qt::Horizontal,2)) {
    QCOMPARE(hp.second, 1);
    for (int i=0; i<hp.second; ++i) {
      column2_headers << hp.first;
    }
  }
  int total = 0;
  for (int r = 0; r < datacube.row_count(); ++r) {
    for (int c = 0; c < datacube.column_count(); ++c) {
      QList<int> elements = datacube.elements(r,c);
      Q_FOREACH(int cell, elements) {
        ++total;
        QCOMPARE(tmp_model->data(tmp_model->index(cell, SEX)).toString(), column0_headers.at(c));
        QCOMPARE(tmp_model->data(tmp_model->index(cell, FIRST_NAME)).toString(), column1_headers.at(c));
        QCOMPARE(tmp_model->data(tmp_model->index(cell, KOMMUNE)).toString(), column2_headers.at(c));
      }
    }
  }
  QCOMPARE(total, 101);


}
void testplaincube::test_add_category_simple()
{
  QStandardItemModel* tmp_model = copy_model();
  std::tr1::shared_ptr<abstract_aggregator_t> sex_aggregator(new column_aggregator_t(tmp_model, SEX));
  std::tr1::shared_ptr<abstract_aggregator_t> kommune_aggregator(new column_aggregator_t(tmp_model, KOMMUNE));
  std::tr1::shared_ptr<abstract_aggregator_t> first_name_aggregator(new column_aggregator_t(tmp_model, FIRST_NAME));
  std::tr1::shared_ptr<abstract_aggregator_t> last_name_aggregator(new column_aggregator_t(tmp_model, LAST_NAME));
  datacube_t datacube(tmp_model, kommune_aggregator, sex_aggregator);
  QList<QStandardItem*> row;
  for (int c=0; c<tmp_model->columnCount(); ++c) {
    switch (static_cast<columns_t>(c)) {
      case FIRST_NAME:
        row << new QStandardItem("Andrea");
        break;
      case LAST_NAME:
        row << new QStandardItem("Hansen");
        break;
      case SEX:
        row << new QStandardItem("neither");
        break;
      case AGE:
        row << new QStandardItem("20");
        break;
      case WEIGHT:
        row << new QStandardItem("80");
        break;
      case KOMMUNE:
        row << new QStandardItem("Hellerup");
        break;
      case N_COLUMNS:
        Q_ASSERT(false);
    }
  }
  tmp_model->appendRow(row);
  QCOMPARE(tmp_model->rowCount(), 101);
  QStringList column0_headers;
  typedef QPair<QString, int> header_pair_t;
  Q_FOREACH(header_pair_t hp, datacube.headers(Qt::Horizontal,0)) {
    for (int i=0; i<hp.second; ++i) {
      column0_headers << hp.first;
    }
  }
  int total = 0;
  for (int r = 0; r < datacube.row_count(); ++r) {
    for (int c = 0; c < datacube.column_count(); ++c) {
      QList<int> elements = datacube.elements(r,c);
      Q_FOREACH(int cell, elements) {
        ++total;
        QCOMPARE(tmp_model->data(tmp_model->index(cell, SEX)).toString(), column0_headers.at(c));
      }
    }
  }
  QCOMPARE(total, 101);

}


void testplaincube::test_delete_rows() {
  QStandardItemModel* tmp_model = copy_model();
  std::tr1::shared_ptr<abstract_aggregator_t> sex_aggregator(new column_aggregator_t(tmp_model, SEX));
  std::tr1::shared_ptr<abstract_aggregator_t> kommune_aggregator(new column_aggregator_t(tmp_model, KOMMUNE));
  datacube_t datacube(tmp_model, kommune_aggregator, sex_aggregator);
  tmp_model->removeRows(30, 10);
  QCOMPARE(m_underlying_model->rowCount()-10, tmp_model->rowCount());
  int total = 0;
  QStringList column0_headers;
  typedef QPair<QString, int> header_pair_t;
  Q_FOREACH(header_pair_t hp, datacube.headers(Qt::Horizontal,0)) {
    for (int i=0; i<hp.second; ++i) {
      column0_headers << hp.first;
    }
  }
  QStringList row0_headers;
  Q_FOREACH(header_pair_t hp, datacube.headers(Qt::Vertical,0)) {
    for (int i=0; i<hp.second; ++i) {
      row0_headers << hp.first;
    }
  }
  for (int r = 0; r < datacube.row_count(); ++r) {
    for (int c = 0; c < datacube.column_count(); ++c) {
      QList<int> elements = datacube.elements(r,c);
      Q_FOREACH(int cell, elements) {
        ++total;
        QCOMPARE(tmp_model->data(tmp_model->index(cell, SEX)).toString(), column0_headers.at(c));
        QCOMPARE(tmp_model->data(tmp_model->index(cell, KOMMUNE)).toString(), row0_headers.at(r));
      }
    }
  }
  QCOMPARE(total, tmp_model->rowCount());

}

void testplaincube::test_add_rows() {
  QStandardItemModel* tmp_model = copy_model();
  std::tr1::shared_ptr<abstract_aggregator_t> sex_aggregator(new column_aggregator_t(tmp_model, SEX));
  std::tr1::shared_ptr<abstract_aggregator_t> kommune_aggregator(new column_aggregator_t(tmp_model, KOMMUNE));
  datacube_t datacube(tmp_model, kommune_aggregator, sex_aggregator);
  for (int i=0; i<10; ++i) {
    int source_row = i*7+2;
    QList<QStandardItem*> newrow;
    for (int c=0; c<tmp_model->columnCount(); ++c) {
      newrow << new QStandardItem(tmp_model->item(source_row, c)->text());
    }
    int dest_row = i*6+7;
    tmp_model->insertRow(dest_row, newrow);
  }
  QCOMPARE(m_underlying_model->rowCount()+10, tmp_model->rowCount());
  int total = 0;
  QStringList column0_headers;
  typedef QPair<QString, int> header_pair_t;
  Q_FOREACH(header_pair_t hp, datacube.headers(Qt::Horizontal,0)) {
    for (int i=0; i<hp.second; ++i) {
      column0_headers << hp.first;
    }
  }
  QStringList row0_headers;
  Q_FOREACH(header_pair_t hp, datacube.headers(Qt::Vertical,0)) {
    for (int i=0; i<hp.second; ++i) {
      row0_headers << hp.first;
    }
  }
  for (int r = 0; r < datacube.row_count(); ++r) {
    for (int c = 0; c < datacube.column_count(); ++c) {
      QList<int> elements = datacube.elements(r,c);
      Q_FOREACH(int cell, elements) {
        ++total;
        QCOMPARE(tmp_model->data(tmp_model->index(cell, SEX)).toString(), column0_headers.at(c));
        QCOMPARE(tmp_model->data(tmp_model->index(cell, KOMMUNE)).toString(), row0_headers.at(r));
      }
    }
  }
  QCOMPARE(total, tmp_model->rowCount());

}


QTEST_MAIN(testplaincube)

#include "testplaincube.moc"
