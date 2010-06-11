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

    void slot_horizontal_context_menu(const QPoint& pos, int headerno, int category);
    void slot_vertical_context_menu(const QPoint& pos, int headerno, int category);
  private:
    qdatacube::datacube_model_t* m_model;
    QTableView* m_view;
    void add_global_filter_bottoms(std::tr1::shared_ptr<qdatacube::abstract_filter_t> filter, QLayout* layout);
    QAction* create_filter_action(std::tr1::shared_ptr< qdatacube::abstract_filter_t > filter);
    QList<QAction*> m_used_filter_actions;
    QList<QAction*> m_unused_filter_actions;

};


#endif // TESTHEADERS_H
