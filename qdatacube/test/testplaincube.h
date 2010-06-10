/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef TESTPLAINCUBE_H
#define TESTPLAINCUBE_H

#include <QObject>
#include <QtTest/QtTest>
#include "danishnamecube.h"

class testplaincube : public danishnamecube_t {
  Q_OBJECT
  public:
    testplaincube(QObject* parent = 0);
  private Q_SLOTS:
    /**
     * Test the column filter
     */
    void test_columnfilter();

    /**
     * Test a basic 2dim datacube
     */
    void test_basics();

    /**
     * Test spliiting a cube
     */
    void test_split();

    /**
     * Test that empty rows are collapsed
     */
    void test_collapse();

    /**
     * Test headers for 3-deep
     */
    void test_deep_header();

    /**
     * Test global filter
     */
    void test_global_filter();
};

#endif // TESTPLAINCUBE_H
