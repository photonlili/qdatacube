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
    void test_basics();
};

#endif // TESTPLAINCUBE_H
