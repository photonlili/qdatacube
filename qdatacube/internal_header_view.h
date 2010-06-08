/*
 Author: Ange Optimization <contact@ange.dk>  (C) Ange Optimization ApS 2010

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef INTERNAL_HEADER_VIEW_H
#define INTERNAL_HEADER_VIEW_H

#include <QHeaderView>

/**
 * This class is the individual header lines/columns. 1 or more of
 * these are put into a QBoxLayout to form the header views datacube_header_t
 */
class internal_header_view_t : public QHeaderView
{
  public:
    /**
     * Constructor, forwarded from parent
     */
    internal_header_view_t(Qt::Orientation orientation, QWidget* parent = 0);

    /**
     * @returns (0,0).
     */
    virtual QSize minimumSizeHint() const;
};

#endif // INTERNAL_HEADER_VIEW_H
