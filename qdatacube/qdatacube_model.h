/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef QDATACUBE_QDATACUBE_MODEL_H
#define QDATACUBE_QDATACUBE_MODEL_H

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QList>

#include "qdatacube_export.h"

namespace qdatacube {

class qdatacube_t;

/**
 * Bridge between the qdatacube model and Qt's model-view framework
 */
class QDATACUBE_EXPORT qdatacube_model_t : public QAbstractTableModel {
    Q_OBJECT
  public:
    qdatacube_model_t(qdatacube_t* datacube, QObject* parent = 0);
    ~qdatacube_model_t();
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    /**
     * The qdatacube model behind the model
     */
    qdatacube_t* qdatacube() const {
      return m_qdatacube;
    }
  signals:
    /**
     * The selection model is on document (but belongs in the view). The qdatacube is in the view (where it belongs)
     * so use this signal to transport between view and document until the selection thing is straightened out.
     */
    void changed();
  public slots:
    /**
     * remove this container from view
     */
    void remove(int container_index);

    /**
     * Adds this container to view
     */
    void readd(int container_index);

    /**
     * Only show the container indexes in list
     */
    void restrict_to_indexes(QList< int > list);
    void slot_begin_remove_column(int);
    void slot_begin_remove_row(int);
    void slot_remove_column(int);
    void slot_remove_row(int);
    void slot_data_changed(int,int);

  private slots:
    void slot_qdatacube_changed();

  private:
    qdatacube_t* m_qdatacube;
};

}

#endif // QDATACUBE_QDATACUBE_MODEL_H
