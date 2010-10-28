/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef DATACUBE_VIEW_H
#define DATACUBE_VIEW_H

#include <QtGui/QTableView>
#include "qdatacube_export.h"


namespace qdatacube {

class datacube_header_t;


class datacube_selection_model_t;


class datacube_view_private_t;

class QDATACUBE_EXPORT datacube_view_t : public QTableView {
  Q_OBJECT
  public:
    datacube_view_t(QWidget* parent = 0);
    virtual ~datacube_view_t();

    /**
     * set the datacube_selection_model_t
     */
    void setSelectionModel(qdatacube::datacube_selection_model_t* selection_model);

    /**
     * Non-virtual override
     * @return vertical header
     */
    datacube_header_t* verticalHeader() const;

    /**
     * Non-virtual override
     * @return horizontal header
     */
    datacube_header_t* horizontalHeader() const;

  private:
    /**
     * @overridden to avoid hiding
     */
    virtual void setSelectionModel(QItemSelectionModel* selectionModel);

    /**
     * Declared private to avoid accident
     */
    void setVerticalHeader(datacube_header_t* headerview);

    /**
     * Declared private to avoid accident
     */
    void setHorizontalHeader(datacube_header_t* headerview);

  QSharedDataPointer<datacube_view_private_t> d;

};

}
#endif // DATACUBE_VIEW_H
