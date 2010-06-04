/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "testplaincube.h"

void testplaincube::test_basics() {
}

testplaincube::testplaincube(QObject* parent): QObject(parent) {
}

QTEST_MAIN(testplaincube)

#include "testplaincube.moc"
