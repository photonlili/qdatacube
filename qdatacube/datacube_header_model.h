
/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef Q_DATACUBE_DATACUBE_HEADER_MODEL_H
#define Q_DATACUBE_DATACUBE_HEADER_MODEL_H

#include <QAbstractItemModel>

namespace qdatacube {

class datacube_model_t;

/**
 * Most of this class is just dummy implementation of the interface, as only headerData is actually used
 */
class datacube_header_model_t : public QAbstractItemModel {
  Q_OBJECT
  public:
    datacube_header_model_t(datacube_model_t* qdatacube_model, Qt::Orientation orientation, int index);

    virtual QVariant data(const QModelIndex& /*index*/, int /*role*/ = Qt::DisplayRole) const {
      return QVariant();
    }

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& /*child*/) const {
      return QModelIndex();
    }
    virtual QModelIndex index(int /*row*/, int /*column*/, const QModelIndex& /*parent*/ = QModelIndex()) const {
      return QModelIndex();
    }
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  private Q_SLOTS:
    void reset_headers();
  private:
    class secret_t;
    QScopedPointer<secret_t> d;
};

}

#endif // Q_DATACUBE_DATACUBE_HEADER_MODEL_H
