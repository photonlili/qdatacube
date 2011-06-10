/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "danishnamecube.h"
#include <QTextStream>
#include <QFile>
#include <QStandardItemModel>
#include <QDebug>

#include "column_aggregator.h"
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
  qDebug() << "Read " << m_underlying_model->rowCount() << " rows" << "with" << m_underlying_model->columnCount() << "columns";

}

danishnamecube_t::danishnamecube_t(QObject* parent):
    QObject(parent),
    m_underlying_model(new QStandardItemModel(0, N_COLUMNS, this)),
    first_name_aggregator(new column_aggregator_t(m_underlying_model, FIRST_NAME)),
    last_name_aggregator(new column_aggregator_t(m_underlying_model, LAST_NAME)),
    sex_aggregator(new column_aggregator_t(m_underlying_model, SEX)),
    age_aggregator(new column_aggregator_t(m_underlying_model, AGE)),
    weight_aggregator(new column_aggregator_t(m_underlying_model, WEIGHT)),
    kommune_aggregator(new column_aggregator_t(m_underlying_model, KOMMUNE))
{
}

QStandardItemModel* danishnamecube_t::copy_model() {
  QStandardItemModel* rv = new QStandardItemModel(0, N_COLUMNS, m_underlying_model);
  for (int r=0; r<m_underlying_model->rowCount(); ++r) {
    QList<QStandardItem*> row;
    for (int c=0; c<m_underlying_model->columnCount(); ++c) {
      row << new QStandardItem(m_underlying_model->item(r,c)->text());
    }
    rv->appendRow(row);
  }
  QStringList header_labels;
  for (int c=0; c<m_underlying_model->columnCount(); ++c) {
    header_labels  << m_underlying_model->headerData(c, Qt::Horizontal).toString();
  }
  rv->setHorizontalHeaderLabels(header_labels);
  return rv;
};

#include "danishnamecube.moc"
