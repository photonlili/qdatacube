/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef TESTDATACUBE_MODEL_H
#define TESTDATACUBE_MODEL_H

#include <QObject>
#include <tr1/memory>
#include "abstract_filter.h"
#include <QAbstractItemModel>

class QStandardItemModel;
namespace qdatacube {
class datacube_t;
}

using namespace qdatacube;

class QAbstractItemModel;

class testdatacube_model : public QObject {
  Q_OBJECT
  public:
    testdatacube_model(QObject* parent = 0);
  public Q_SLOTS:
    void columnsAboutToBeRemoved ( const QModelIndex & parent, int start, int end );
    void columnsRemoved ( const QModelIndex & parent, int start, int end );
    void columnsAboutToBeAdded ( const QModelIndex & parent, int start, int end );
    void columnsAdded ( const QModelIndex & parent, int start, int end );
    void rowsAboutToBeRemoved ( const QModelIndex & parent, int start, int end );
    void rowsRemoved ( const QModelIndex & parent, int start, int end );
    void rowsAboutToBeAdded ( const QModelIndex & parent, int start, int end );
    void rowsAdded ( const QModelIndex & parent, int start, int end );
  private Q_SLOTS:
    void testplain();
    void testdatachange();
  private:
    QStandardItemModel* m_model;
    datacube_t* m_cube;
    std::tr1::shared_ptr<abstract_filter_t> first_name_filter;
    std::tr1::shared_ptr<abstract_filter_t> last_name_filter;
    std::tr1::shared_ptr<abstract_filter_t> sex_filter;
    std::tr1::shared_ptr<abstract_filter_t> age_filter;
    std::tr1::shared_ptr<abstract_filter_t> weight_filter;
    std::tr1::shared_ptr<abstract_filter_t> kommune_filter;
    void clear_rowcol_changed();
    void connect_rowcol_changed(QAbstractItemModel* model);
    QList<int> m_rows_removed;
    QList<int> m_columns_removed;
    QList<int> m_rows_added;
    QList<int> m_columns_added;
    enum columns_t {
      FIRST_NAME,
      LAST_NAME,
      SEX,
      AGE,
      WEIGHT,
      KOMMUNE,
      N_COLUMNS
    };

};

#endif // TESTDATACUBE_MODEL_H
