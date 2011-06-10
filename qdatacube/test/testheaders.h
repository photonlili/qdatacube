/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef TESTHEADERS_H
#define TESTHEADERS_H

#include <QObject>
#include "danishnamecube.h"

class QAction;
class QPoint;
class QLayout;
class QStandardItemModel;
class QTableView;
namespace qdatacube {
class datacube_model_t;
class datacube_view_t;
}

class QAbstractItemModel;
class testheaders : public danishnamecube_t {
  Q_OBJECT
  public:
    void createtableview();
    testheaders(QObject* parent = 0);
  public Q_SLOTS:
    void slot_set_model();
    void slot_set_filter();
    void slot_set_data();
    void slot_insert_data();
    void slot_remove_data();
    void slot_global_filter_button_pressed();

    void slot_horizontal_context_menu(QPoint pos, int headerno, int category);
    void slot_vertical_context_menu(const QPoint& pos, int headerno, int category);
  private:
    qdatacube::datacube_t* m_datacube;
    qdatacube::datacube_view_t* m_view;
    void add_global_filter_bottoms(std::tr1::shared_ptr< qdatacube::abstract_aggregator_t > filter, QLayout* layout);
    QAction* create_filter_action(std::tr1::shared_ptr< qdatacube::abstract_aggregator_t > filter);
    QAction* m_collapse_action;
    QList<QAction*> m_col_used_filter_actions;
    QList<QAction*> m_row_used_filter_actions;
    QList<QAction*> m_unused_filter_actions;
    QTableView* m_underlying_table_view;

};


#endif // TESTHEADERS_H
