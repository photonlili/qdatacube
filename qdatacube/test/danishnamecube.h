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
class Datacube;
class AbstractAggregator;
}

#include <columnaggregator.h>
#include <QColor>
class SexAggregator : public qdatacube::ColumnAggregator {
    public:
        SexAggregator(QAbstractItemModel* model, int section) : ColumnAggregator(model,section) {

        }
        virtual QVariant categoryHeaderData(int category, int role = Qt::DisplayRole) const {
            QVariant displayrole = ColumnAggregator::categoryHeaderData(category,Qt::DisplayRole);
            if(role == Qt::DisplayRole) {
                return displayrole;
            }
            if(role == Qt::BackgroundRole) {
                QString displayrolestring = displayrole.toString();
                if(displayrolestring == "male") {
                    return QColor(Qt::cyan);
                } else if(displayrolestring == "female") {
                    return QColor(Qt::magenta);
                }
                return QColor(Qt::yellow);
            }
            if(role == Qt::ForegroundRole) {
                QString displayrolestring = displayrole.toString();
                if(displayrolestring == "male") {
                    return QColor(Qt::blue);
                } else if(displayrolestring == "female") {
                    return QColor(Qt::red);
                }
                return QColor(Qt::gray);

            }
            return QVariant();
        }
};


class danishnamecube_t : public QObject {
  Q_OBJECT
  protected:
    danishnamecube_t(QObject* parent = 0);
    static int printdatacube(const qdatacube::Datacube* datacube);
    void load_model_data(QString filename);
    /**
     * Return a deep copy of the underlying model (to test manipulation functions)
     */
    QStandardItemModel* copy_model();
    QStandardItemModel* m_underlying_model;
    std::tr1::shared_ptr<qdatacube::AbstractAggregator> first_name_aggregator;
    std::tr1::shared_ptr<qdatacube::AbstractAggregator> last_name_aggregator;
    std::tr1::shared_ptr<qdatacube::AbstractAggregator> sex_aggregator;
    std::tr1::shared_ptr<qdatacube::AbstractAggregator> age_aggregator;
    std::tr1::shared_ptr<qdatacube::AbstractAggregator> weight_aggregator;
    std::tr1::shared_ptr<qdatacube::AbstractAggregator> kommune_aggregator;
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
