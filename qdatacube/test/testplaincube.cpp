/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "testplaincube.h"
#include <QStandardItemModel>
#include "qdatacube/datacube.h"

using namespace qdatacube;

void testplaincube::test_basics() {
//  datacube_t datacube(m_model);
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
