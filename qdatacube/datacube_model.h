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

  Q_SIGNALS:
    /**
     * The selection model is on document (but belongs in the view). The qdatacube is in the view (where it belongs)
     * so use this signal to transport between view and document until the selection thing is straightened out.
     */
    void changed();
  private Q_SLOTS:
    void slot_begin_remove_column(int);
    void slot_begin_remove_row(int);
    void slot_remove_column(int);
    void slot_remove_row(int);
    void slot_data_changed(int,int);

    void slot_datacube_changed();

  private:
    class secret_t;
    QScopedPointer<secret_t> d;
};

}

#endif // Q_DATACUBE_DATACUBE_MODEL_H
