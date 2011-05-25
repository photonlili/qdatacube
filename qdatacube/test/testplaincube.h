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

  private:
    void dotest_splittwice(Qt::Orientation direction);

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
    * Test spliiting a cube
    */
    void test_split_twice_horizontal();

    /**
    * Test spliiting a cube
    */
    void test_split_twice_vertical();

    /**
     * Test collapsing sections (first of 2 headers)
     */
    void test_collapse1();

    /**
     * Test collapsing sections (last of 2 headers)
     */
    void test_collapse2();

    /**
     * Test collapsing sections (middle of 3 headers)
     */
    void test_collapse3();

    /**
     * Test colapsing section, vertically
     */
    void test_collapse3_vertical();

    /**
     * Test that empty rows are collapsed
     */
    void test_autocollapse();

    /**
     * Test headers for 3-deep
     */
    void test_deep_header();

    /**
     * Test global filter
     */
    void test_global_filter();

    /**
     * Test reverse index
     */
    void test_reverse_index();

    /**
    * Test add category to a filter
    */
    void test_add_category();

    /**
    * Test add category to a filter
    */
    void test_add_category_simple();

    /**
    * Test remove category from a filter;
    */
//    void test_remove_category(); // TODO

    /**
     * Test section_for_element_internal
     */
    void test_section_for_element_internal();

    void do_testcollapse3(Qt::Orientation orientation);

    /**
     * Test delete model rows (in the middle)
     **/
    void test_delete_rows();

    /**
     * Test add model rows (in the middle)
     **/
    void test_add_rows();
};

#endif // TESTPLAINCUBE_H
