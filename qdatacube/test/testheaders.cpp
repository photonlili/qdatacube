/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#include "testheaders.h"
#include "column_filter.h"
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

using namespace qdatacube;

testheaders::testheaders(QObject* parent) : danishnamecube_t(parent), m_underlying_table_view(0L) {
  load_model_data("plaincubedata.txt");

  m_datacube = new datacube_t(m_underlying_model, first_name_filter, last_name_filter);
  m_row_used_filter_actions << create_filter_action(first_name_filter);
  m_col_used_filter_actions << create_filter_action(last_name_filter);
  m_unused_filter_actions << create_filter_action(sex_filter) << create_filter_action(age_filter) << create_filter_action(weight_filter) << create_filter_action(kommune_filter);
  m_collapse_action = new QAction("Collapse", this);
  m_unused_filter_actions << m_collapse_action;

}

QAction* testheaders::create_filter_action(std::tr1::shared_ptr< abstract_filter_t > filter)
{
  QAction* rv = new QAction(filter->name(m_underlying_model), this);
  rv->setData(QVariant::fromValue(static_cast<void*>(filter.get())));
  return rv;
}

void testheaders::add_global_filter_bottoms(std::tr1::shared_ptr< abstract_filter_t > filter, QLayout* layout) {
  QWidget* top = new QWidget(layout->widget());
  layout->addWidget(top);
  QBoxLayout* lay = new QVBoxLayout(top);
  int categoryno = 0;
  Q_FOREACH(QString cat, filter->categories(m_underlying_model)) {
    QPushButton* button = new QPushButton(cat, top);
    column_filter_t* cf = static_cast<column_filter_t*>(filter.get());
    button->setProperty("section", cf->section());
    button->setProperty("categoryno", categoryno++);
    lay->addWidget(button);
    connect(button, SIGNAL(clicked(bool)), SLOT(slot_global_filter_button_pressed()));
  }
  lay->addStretch();

}

void testheaders::slot_global_filter_button_pressed() {
  QObject* s = sender();
  if (s->property("clear").toBool()) {
    m_datacube->reset_global_filter();
  } else {
    int section = s->property("section").toInt();
    int categoryno = s->property("categoryno").toInt();
    m_datacube->set_global_filter(new column_filter_t(section), categoryno);
  }
}

void testheaders::createtableview() {
  QMainWindow* top = new QMainWindow();
  QWidget* mw = new QWidget();
  top->setCentralWidget(mw);
  new QVBoxLayout(mw);
  m_view = new datacube_view_t(top);
  m_view->set_datacube(m_datacube);
  mw->layout()->addWidget(m_view);
  connect(m_view, SIGNAL(horizontal_header_context_menu(QPoint,int,int)), SLOT(slot_horizontal_context_menu(QPoint,int,int)));
  connect(m_view, SIGNAL(vertical_header_context_menu(QPoint,int,int)), SLOT(slot_vertical_context_menu(QPoint,int,int)));
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
  add_global_filter_bottoms(first_name_filter, gfml);
  add_global_filter_bottoms(last_name_filter, gfml);
  add_global_filter_bottoms(sex_filter, gfml);
  add_global_filter_bottoms(age_filter, gfml);
  add_global_filter_bottoms(weight_filter, gfml);
  add_global_filter_bottoms(kommune_filter, gfml);

  QDockWidget* underlying_view = new QDockWidget("Underlying model", top);
  top->addDockWidget(Qt::RightDockWidgetArea, underlying_view);
  m_underlying_table_view = new QTableView(underlying_view);
  QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
  proxy->setSourceModel(m_underlying_model);
  m_underlying_table_view->setModel(proxy);
  m_underlying_table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  underlying_view->setWidget(m_underlying_table_view);
  m_underlying_table_view->setSortingEnabled(true);
  m_view->datacube_selection()->synchronize_with(m_underlying_table_view->selectionModel());


  QDockWidget* second_dc = new QDockWidget("Second datacube");
  top->addDockWidget(Qt::BottomDockWidgetArea, second_dc);
  datacube_view_t* second_view = new datacube_view_t(second_dc);
  datacube_t* second_datacube = new datacube_t(m_underlying_model, kommune_filter, age_filter);
  second_view->set_datacube(second_datacube);
  second_view->datacube_selection()->synchronize_with(m_underlying_table_view->selectionModel());
  second_dc->setWidget(second_view);
}

void testheaders::slot_set_model() {
  m_view->set_datacube(m_datacube);
  m_view->datacube_selection()->synchronize_with(m_underlying_table_view->selectionModel());

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
  std::tr1::shared_ptr<abstract_filter_t> filter(new column_filter_t(SEX));
  m_datacube->set_global_filter(filter, (count++%2));
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
  QList<QAction*> actions = m_unused_filter_actions;
  if (headerno<0) {
    actions.removeAll(m_collapse_action);
  }
  QAction* action = QMenu::exec(actions, QCursor::pos());
  if (action) {
    if (action != m_collapse_action) {
      abstract_filter_t* raw_pointer = static_cast<abstract_filter_t*>(action->data().value<void*>());
      std::tr1::shared_ptr<abstract_filter_t> filter;
      if (first_name_filter.get() == raw_pointer) {
        filter = first_name_filter;
      } else if (last_name_filter.get() == raw_pointer) {
        filter = last_name_filter;
      } else if (sex_filter.get() == raw_pointer) {
        filter = sex_filter;
      } else if (age_filter.get() == raw_pointer) {
        filter = age_filter;
      } else if (weight_filter.get() == raw_pointer) {
        filter = weight_filter;
      } else if (kommune_filter.get() == raw_pointer) {
        filter = kommune_filter;
      }
      m_datacube->split(Qt::Horizontal, qMax(headerno,0), filter);
      m_unused_filter_actions.removeAll(action);
      m_col_used_filter_actions.insert(headerno, action);
    } else {
      m_datacube->collapse(Qt::Horizontal, headerno);
      QAction* filter_acton = m_col_used_filter_actions.takeAt(headerno);
      m_unused_filter_actions << filter_acton;
    }
  }

}

void testheaders::slot_vertical_context_menu(const QPoint& /*pos*/, int headerno, int /*category*/) {
  QList<QAction*> actions = m_unused_filter_actions;
  if (headerno<0) {
    actions.removeAll(m_collapse_action);
  }
  QAction* action = QMenu::exec(actions, QCursor::pos());
  if (action) {
    if (action != m_collapse_action) {
      abstract_filter_t* raw_pointer = static_cast<abstract_filter_t*>(action->data().value<void*>());
      std::tr1::shared_ptr<abstract_filter_t> filter;
      if (first_name_filter.get() == raw_pointer) {
        filter = first_name_filter;
      } else if (last_name_filter.get() == raw_pointer) {
        filter = last_name_filter;
      } else if (sex_filter.get() == raw_pointer) {
        filter = sex_filter;
      } else if (age_filter.get() == raw_pointer) {
        filter = age_filter;
      } else if (weight_filter.get() == raw_pointer) {
        filter = weight_filter;
      } else if (kommune_filter.get() == raw_pointer) {
        filter = kommune_filter;
      }
      if (headerno+1 < m_datacube->header_count(Qt::Vertical)) {
        m_row_used_filter_actions << action;
      } else {
        m_row_used_filter_actions.insert(headerno+1, action);
      }
      m_datacube->split(Qt::Vertical, headerno+1, filter);
      m_unused_filter_actions.removeAll(action);
    } else {
      m_datacube->collapse(Qt::Vertical, headerno);
      QAction* filter_acton = m_row_used_filter_actions.takeAt(headerno);
      m_unused_filter_actions << filter_acton;
    }
  }

}
#include "testheaders.moc"
