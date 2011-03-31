/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "danishnamecube.h"
#include <QTextStream>
#include <QFile>
#include <QStandardItemModel>
#include <QDebug>

#include "column_filter.h"
#include "datacube.h"

using namespace qdatacube;

int danishnamecube_t::printdatacube(const qdatacube::datacube_t* datacube) {
  qDebug() << datacube->column_count() << ", " << datacube->row_count();
  qDebug() << datacube->header_count(Qt::Horizontal) << ", " << datacube->header_count(Qt::Vertical);
  qDebug() << "column headers";
  for (int i=0; i< datacube->header_count(Qt::Horizontal); ++i) {
    qDebug() << datacube->headers(Qt::Horizontal, i);
  }
  qDebug() << "row headers";
  for (int i=0; i< datacube->header_count(Qt::Vertical); ++i) {
    qDebug() << datacube->headers(Qt::Vertical, i);
  }
  int total = 0;
  for (int r = 0; r< datacube->row_count(); ++r) {
    QString row_display;
    QTextStream row_display_stream(&row_display);
    for (int c = 0; c< datacube->column_count(); ++c) {
      int count = datacube->element_count(r,c);
      total += count;
      row_display_stream << datacube->element_count(r,c) << "\t";
    }
    qDebug() << row_display;
  }

  return total;
}

void danishnamecube_t::load_model_data(QString filename) {
  QFile data(QString(DATADIR)+"/"+filename);
  data.open(QIODevice::ReadOnly);
  m_underlying_model = new QStandardItemModel(0, N_COLUMNS, this);
  while (!data.atEnd()) {
    QString line = QString::fromLocal8Bit(data.readLine());
    QStringList columns = line.split(' ');
    Q_ASSERT(columns.size() == N_COLUMNS);
    QList<QStandardItem*> cell_items;
    Q_FOREACH(QString cell, columns) {
      cell.remove("\n");
      cell_items << new QStandardItem(cell);
    }
    m_underlying_model->appendRow(cell_items);
  }
  QStringList labels;
  labels << "firstname" << "lastname" << "sex" << "age" << "weight" << "kommune";
  m_underlying_model->setHorizontalHeaderLabels(labels);
  qDebug() << "Read " << m_underlying_model->rowCount() << " rows";

}

danishnamecube_t::danishnamecube_t(QObject* parent):
    QObject(parent),
    m_underlying_model(new QStandardItemModel(this)),
    first_name_filter(new column_filter_t(FIRST_NAME)),
    last_name_filter(new column_filter_t(LAST_NAME)),
    sex_filter(new column_filter_t(SEX)),
    age_filter(new column_filter_t(AGE)),
    weight_filter(new column_filter_t(WEIGHT)),
    kommune_filter(new column_filter_t(KOMMUNE))
{
}

#include "danishnamecube.moc"
