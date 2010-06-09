/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef TESTHEADERS_H
#define TESTHEADERS_H

#include <QObject>

class QTableView;
namespace qdatacube {
class datacube_model_t;
}

class QAbstractItemModel;
class testheaders : public QObject {
  Q_OBJECT
  public:
    void createtableview();
    testheaders(QObject* parent = 0);
    enum columns_t {
      FIRST_NAME,
      LAST_NAME,
      SEX,
      AGE,
      WEIGHT,
      KOMMUNE,
      N_COLUMNS

    };
  public Q_SLOTS:
    void slot_set_model();
    void slot_set_filter();
  private:
    qdatacube::datacube_model_t* m_model;
    QAbstractItemModel* m_underlying_model;
    QTableView* m_view;
};


#endif // TESTHEADERS_H
