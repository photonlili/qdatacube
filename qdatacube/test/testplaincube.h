/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef TESTPLAINCUBE_H
#define TESTPLAINCUBE_H

#include <QObject>
#include <QtTest/QtTest>

class testplaincube : public QObject {
  Q_OBJECT
  public:
    testplaincube(QObject* parent = 0);
  private Q_SLOTS:
    void test_columnfilter();
    void test_basics();
  private:
    QAbstractItemModel* m_model;
    enum columns_t {
      FIRST_NAME,
      LAST_NAME,
      SEX,
      AGE,
      WEIGHT,
      KOMMUNE,
      N_COLUMNS

    };
};

#endif // TESTPLAINCUBE_H
