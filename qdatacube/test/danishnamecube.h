/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef DANISHNAMECUBE_H
#define DANISHNAMECUBE_H

#include <QObject>
#include <tr1/memory>

class QStandardItemModel;
class QAbstractItemModel;
namespace qdatacube {
class datacube_t;
class abstract_filter_t;
}
class danishnamecube_t : public QObject {
  Q_OBJECT
  protected:
    danishnamecube_t(QObject* parent = 0);
    static int printdatacube(const qdatacube::datacube_t* datacube);
    void load_model_data(QString filename);
    QStandardItemModel* m_underlying_model;
    std::tr1::shared_ptr<qdatacube::abstract_filter_t> first_name_filter;
    std::tr1::shared_ptr<qdatacube::abstract_filter_t> last_name_filter;
    std::tr1::shared_ptr<qdatacube::abstract_filter_t> sex_filter;
    std::tr1::shared_ptr<qdatacube::abstract_filter_t> age_filter;
    std::tr1::shared_ptr<qdatacube::abstract_filter_t> weight_filter;
    std::tr1::shared_ptr<qdatacube::abstract_filter_t> kommune_filter;
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

#endif // DANISHNAMECUBE_H
