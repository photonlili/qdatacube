/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "testheaders.h"
#include "column_filter.h"
#include "datacube.h"
#include "datacube_colrow.h"
#include "datacube_model.h"
#include "datacube_header.h"
#include <QStandardItemModel>
#include <QFile>
#include <QTableView>
#include <QVBoxLayout>
#include <QApplication>
#include <QDockWidget>
#include <QMainWindow>
#include <QLabel>
#include <QTimer>

using namespace qdatacube;

testheaders::testheaders(QObject* parent) : QObject(parent) {
  QFile data(DATADIR "/plaincubedata.txt");
  data.open(QIODevice::ReadOnly);
  QStandardItemModel* model = new QStandardItemModel(0, N_COLUMNS, this);
  while (!data.atEnd()) {
    QString line = QString::fromLocal8Bit(data.readLine());
    QStringList columns = line.split(' ');
    Q_ASSERT(columns.size() == N_COLUMNS);
    QList<QStandardItem*> cell_items;
    Q_FOREACH(QString cell, columns) {
      cell.remove("\n");
      cell_items << new QStandardItem(cell);
    }
    model->appendRow(cell_items);
  }
  m_underlying_model = model;
  qDebug() << "Read " << m_underlying_model->rowCount() << " rows";

  QList<int> allactive;
  const int nrows = m_underlying_model->rowCount();
  for (int i=0; i<nrows;++i) {
    allactive << i;
  }
  column_filter_t* firstname_filter = new column_filter_t(0);
  column_filter_t* lastname_filter = new column_filter_t(1);
  std::tr1::shared_ptr<abstract_filter_t> sex_filter(new column_filter_t(2));
  datacube_t* datacube = new datacube_t(m_underlying_model, firstname_filter, lastname_filter, allactive);
  datacube->toplevel_row_header().split(sex_filter);
  datacube->toplevel_column_header().split(sex_filter);
  m_model = new datacube_model_t(datacube);

}

void testheaders::createtableview() {
  QMainWindow* top = new QMainWindow();
//   top->setCentralWidget(new QLabel("Central", top));
/*  QDockWidget* dock = new QDockWidget("dock");
  top->addDockWidget(Qt::LeftDockWidgetArea,dock);*/
  QWidget* mw = new QWidget();
  top->setCentralWidget(mw);
  new QVBoxLayout(mw);
  m_view = new QTableView;
  mw->layout()->addWidget(m_view);
  m_view->setHorizontalHeader(new datacube_header_t(Qt::Horizontal, m_view));
  m_view->setVerticalHeader(new datacube_header_t(Qt::Vertical, m_view));
  top->resize(1600, 1000);
  top->show();
  QTimer::singleShot(500, this, SLOT(slot_set_model()));
 }

void testheaders::slot_set_model() {
  m_view->setModel(m_model);
  m_view->resizeColumnsToContents();
  m_view->resizeRowsToContents();

}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setFont(QFont("serif", 5));
  testheaders test;
  test.createtableview();
  qDebug() << "Starting now!";
  app.exec();
}

#include "testheaders.moc"