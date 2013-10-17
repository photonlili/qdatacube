/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "testheaders.h"
#include "column_aggregator.h"
#include "datacube.h"
#include <QStandardItemModel>
#include <QFile>
#include <QTableView>
#include <QVBoxLayout>
#include <QApplication>
#include <QDockWidget>
#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QScrollArea>
#include <QMenu>
#include <datacube_view.h>
#include <datacube_selection.h>
#include <qsortfilterproxymodel.h>
#include <filter_by_aggregate.h>
#include <qmenubar.h>
#include "column_sum_formatter.h"
#include <count_formatter.h>

using namespace qdatacube;

testheaders::testheaders(QObject* parent) : danishnamecube_t(parent), m_underlying_table_view(0L) {
  load_model_data("plaincubedata.txt");

  m_datacube = new Datacube(m_underlying_model, first_name_aggregator, last_name_aggregator);
  m_row_used_aggregator_actions << create_aggregator_action(first_name_aggregator);
  m_col_used_aggregator_actions << create_aggregator_action(last_name_aggregator);
  m_unused_aggregator_actions << create_aggregator_action(sex_aggregator) << create_aggregator_action(age_aggregator) << create_aggregator_action(weight_aggregator) << create_aggregator_action(kommune_aggregator);
  m_collapse_action = new QAction("Collapse", this);
  m_unused_aggregator_actions << m_collapse_action;

}

QAction* testheaders::create_aggregator_action(std::tr1::shared_ptr< AbstractAggregator > aggregator)
{
  QAction* rv = new QAction(aggregator->name(), this);
  rv->setData(QVariant::fromValue(static_cast<void*>(aggregator.get())));
  return rv;
}

void testheaders::add_global_filter_bottoms(std::tr1::shared_ptr< AbstractAggregator > aggregator, QLayout* layout) {
  QWidget* top = new QWidget(layout->widget());
  layout->addWidget(top);
  QBoxLayout* lay = new QVBoxLayout(top);
  for(int cat = 0; cat < aggregator->categoryCount() ; cat++) {
      QString catText = aggregator->categoryHeaderData(cat).toString();
    QPushButton* button = new QPushButton(catText, top);
    ColumnAggregator* cf = static_cast<ColumnAggregator*>(aggregator.get());
    button->setProperty("section", cf->section());
    button->setProperty("categoryno", cat);
    lay->addWidget(button);
    connect(button, SIGNAL(clicked(bool)), SLOT(slot_global_filter_button_pressed()));
  }
  lay->addStretch();

}

void testheaders::slot_global_filter_button_pressed() {
  QObject* s = sender();
  if (s->property("clear").toBool()) {
    m_datacube->resetGlobalFilter();
  } else {
    int section = s->property("section").toInt();
    int categoryno = s->property("categoryno").toInt();
    m_datacube->addGlobalFilter(std::tr1::shared_ptr<AbstractFilter>(new filter_by_aggregate_t(std::tr1::shared_ptr<qdatacube::AbstractAggregator>(new ColumnAggregator(m_underlying_model, section)), categoryno)));
  }
}

void testheaders::createtableview() {
  QMainWindow* top = new QMainWindow();
  QWidget* mw = new QWidget();
  top->setCentralWidget(mw);
  new QVBoxLayout(mw);
  m_view = new DatacubeView(top);
  m_view->addFormatter(new CountFormatter(m_underlying_model, m_view));
  m_view->setDatacube(m_datacube);
  mw->layout()->addWidget(m_view);
  connect(m_view, SIGNAL(horizontalHeaderContextMenu(QPoint,int,int)), SLOT(slot_horizontal_context_menu(QPoint,int,int)));
  connect(m_view, SIGNAL(verticalHeaderContextMenu(QPoint,int,int)), SLOT(slot_vertical_context_menu(QPoint,int,int)));
  top->resize(1600, 1000);
  top->show();
  QTimer::singleShot(500, this, SLOT(slot_set_model()));
  QDockWidget* global_filters = new QDockWidget("Global filters", top);
  top->addDockWidget(Qt::LeftDockWidgetArea,global_filters);
  QWidget* gfcw = new QScrollArea(global_filters);
  global_filters->setWidget(gfcw);
  QBoxLayout* gfml = new QHBoxLayout(gfcw);
  QPushButton*  clear_button = new QPushButton("clear", gfcw);
  clear_button->setProperty("clear", true);
  connect(clear_button, SIGNAL(clicked(bool)), SLOT(slot_global_filter_button_pressed()));
  gfml->addWidget(clear_button);
  add_global_filter_bottoms(first_name_aggregator, gfml);
  add_global_filter_bottoms(last_name_aggregator, gfml);
  add_global_filter_bottoms(sex_aggregator, gfml);
  add_global_filter_bottoms(age_aggregator, gfml);
  add_global_filter_bottoms(weight_aggregator, gfml);
  add_global_filter_bottoms(kommune_aggregator, gfml);

  QDockWidget* underlying_view = new QDockWidget("Underlying model", top);
  top->addDockWidget(Qt::RightDockWidgetArea, underlying_view);
  m_underlying_table_view = new QTableView(underlying_view);
  QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
  proxy->setSourceModel(m_underlying_model);
  m_underlying_table_view->setModel(proxy);
  m_underlying_table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  underlying_view->setWidget(m_underlying_table_view);
  m_underlying_table_view->setSortingEnabled(true);
  m_view->datacubeSelection()->synchronizeWith(m_underlying_table_view->selectionModel());


  QDockWidget* second_dc = new QDockWidget("Second datacube");
  top->addDockWidget(Qt::BottomDockWidgetArea, second_dc);
  DatacubeView* second_view = new DatacubeView(second_dc);
  second_view->addFormatter(new CountFormatter(m_underlying_model, second_view));
  Datacube* second_datacube = new Datacube(m_underlying_model, kommune_aggregator, age_aggregator);
  second_view->setDatacube(second_datacube);
  second_view->datacubeSelection()->synchronizeWith(m_underlying_table_view->selectionModel());
  second_dc->setWidget(second_view);
  QAction* sum_weight = top->menuBar()->addAction("Summarize over weight");
  connect(sum_weight, SIGNAL(triggered(bool)), SLOT(summarize_weight()));
}

void testheaders::slot_set_model() {
  m_view->setDatacube(m_datacube);
  m_view->datacubeSelection()->synchronizeWith(m_underlying_table_view->selectionModel());

}

void testheaders::slot_set_data() {
  QModelIndex index = m_underlying_model->index(15, FIRST_NAME); // Lulu Petersen
  QList<QString> names;
  names << "Andersine" << "Bertrand" << "Lulu";
  static int nameno = 0;
  QString newname = names[nameno++];
  m_underlying_model->setData(index, newname);
  if (nameno == names.size()) {
    nameno = 0;
  }
  QTimer::singleShot(20, this, SLOT(slot_set_data()));


}

void testheaders::slot_set_filter() {
  static int count = 0;
  std::tr1::shared_ptr<AbstractAggregator> aggregator(new ColumnAggregator(m_underlying_model, SEX));
  m_datacube->resetGlobalFilter();
  m_datacube->addGlobalFilter(std::tr1::shared_ptr<AbstractFilter>(new filter_by_aggregate_t(aggregator, (count++%2))));
  QTimer::singleShot(2000, this, SLOT(slot_set_filter()));
}

void testheaders::slot_insert_data() {
  QFile data(DATADIR "/100extra.txt");
  data.open(QIODevice::ReadOnly);
  int n=0;
  while (!data.atEnd()) {
    QString line = QString::fromLocal8Bit(data.readLine());
    QStringList columns = line.split(' ');
    Q_ASSERT(columns.size() == N_COLUMNS);
    QList<QStandardItem*> cell_items;
    Q_FOREACH(QString cell, columns) {
      cell.remove("\n");
      cell_items << new QStandardItem(cell);
    }
    int row = qrand() % (m_underlying_model->rowCount()+1);
    if (row == m_underlying_model->rowCount()) {
      m_underlying_model->appendRow(cell_items);
    } else {
      m_underlying_model->insertRow(row, cell_items);
    }
    ++n;
  }
  qDebug() << "Inserted " << n << " rows";
  QTimer::singleShot(200, this, SLOT(slot_remove_data()));
}

void testheaders::slot_remove_data() {
  const int N = 60;
  if (m_underlying_model->rowCount() < N) {
    return;
  }
  for (int i=0; i<N; ) {
    int row = qrand() % m_underlying_model->rowCount();
    int count = qrand() % std::min(m_underlying_model->rowCount()-row, N);
    m_underlying_model->removeRows(row, count);
    qDebug() << "Removed " << count << " rows";
    i+=count;
  }
  QTimer::singleShot(200, this, SLOT(slot_insert_data()));
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setFont(QFont("serif", 10));
  testheaders test;
  test.createtableview();
  qDebug() << "Starting now!";
  app.exec();
}

void testheaders::slot_horizontal_context_menu(QPoint /*pos*/, int headerno, int /*category*/) {
  QList<QAction*> actions = m_unused_aggregator_actions;
  if (headerno<0) {
    actions.removeAll(m_collapse_action);
  }
  QAction* action = QMenu::exec(actions, QCursor::pos());
  if (action) {
    if (action != m_collapse_action) {
      AbstractAggregator* raw_pointer = static_cast<AbstractAggregator*>(action->data().value<void*>());
      std::tr1::shared_ptr<AbstractAggregator> aggregator;
      if (first_name_aggregator.get() == raw_pointer) {
        aggregator = first_name_aggregator;
      } else if (last_name_aggregator.get() == raw_pointer) {
        aggregator = last_name_aggregator;
      } else if (sex_aggregator.get() == raw_pointer) {
        aggregator = sex_aggregator;
      } else if (age_aggregator.get() == raw_pointer) {
        aggregator = age_aggregator;
      } else if (weight_aggregator.get() == raw_pointer) {
        aggregator = weight_aggregator;
      } else if (kommune_aggregator.get() == raw_pointer) {
        aggregator = kommune_aggregator;
      }
      m_datacube->split(Qt::Horizontal, qMax(headerno,0), aggregator);
      m_unused_aggregator_actions.removeAll(action);
      m_col_used_aggregator_actions.insert(headerno, action);
    } else {
      m_datacube->collapse(Qt::Horizontal, headerno);
      QAction* filter_acton = m_col_used_aggregator_actions.takeAt(headerno);
      m_unused_aggregator_actions << filter_acton;
    }
  }

}

void testheaders::slot_vertical_context_menu(const QPoint& /*pos*/, int headerno, int /*category*/) {
  QList<QAction*> actions = m_unused_aggregator_actions;
  if (headerno<0) {
    actions.removeAll(m_collapse_action);
  }
  QAction* action = QMenu::exec(actions, QCursor::pos());
  if (action) {
    if (action != m_collapse_action) {
      AbstractAggregator* raw_pointer = static_cast<AbstractAggregator*>(action->data().value<void*>());
      std::tr1::shared_ptr<AbstractAggregator> aggregator;
      if (first_name_aggregator.get() == raw_pointer) {
        aggregator = first_name_aggregator;
      } else if (last_name_aggregator.get() == raw_pointer) {
        aggregator = last_name_aggregator;
      } else if (sex_aggregator.get() == raw_pointer) {
        aggregator = sex_aggregator;
      } else if (age_aggregator.get() == raw_pointer) {
        aggregator = age_aggregator;
      } else if (weight_aggregator.get() == raw_pointer) {
        aggregator = weight_aggregator;
      } else if (kommune_aggregator.get() == raw_pointer) {
        aggregator = kommune_aggregator;
      }
      if (headerno+1 < m_datacube->headerCount(Qt::Vertical)) {
        m_row_used_aggregator_actions << action;
      } else {
        m_row_used_aggregator_actions.insert(headerno+1, action);
      }
      m_datacube->split(Qt::Vertical, headerno+1, aggregator);
      m_unused_aggregator_actions.removeAll(action);
    } else {
      m_datacube->collapse(Qt::Vertical, headerno);
      QAction* filter_acton = m_row_used_aggregator_actions.takeAt(headerno);
      m_unused_aggregator_actions << filter_acton;
    }
  }

}

void testheaders::summarize_weight()
{
  ColumnSumFormatter* summarizer = new ColumnSumFormatter(m_underlying_model, m_view, WEIGHT, 1, "t", .001);
  m_view->addFormatter(summarizer);

}

#include "testheaders.moc"
