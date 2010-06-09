/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef Q_DATACUBE_DATACUBE_MODEL_H
#define Q_DATACUBE_DATACUBE_MODEL_H

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QList>

#include "qdatacube_export.h"

namespace qdatacube {

class datacube_t;

/**
 * Bridge between the qdatacube model and Qt's model-view framework
 */
class QDATACUBE_EXPORT datacube_model_t : public QAbstractTableModel {
    Q_OBJECT
  public:
    datacube_model_t(datacube_t* datacube, QObject* parent = 0);
    ~datacube_model_t();
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    /**
     * The datacube model behind the model
     */
    datacube_t* datacube() const;

  private Q_SLOTS:
    void slot_datacube_about_to_remove_column(int);
    void slot_datacube_about_to_remove_row(int);
    void slot_datacube_removed_column(int);
    void slot_datacube_removed_row(int);
    void slot_datacube_about_to_add_column(int);
    void slot_datacube_about_to_add_row(int);
    void slot_datacube_added_column(int);
    void slot_datacube_added_row(int);
    void slot_data_changed(int,int);
  private:
    class secret_t;
    QScopedPointer<secret_t> d;
};

}

#endif // Q_DATACUBE_DATACUBE_MODEL_H
