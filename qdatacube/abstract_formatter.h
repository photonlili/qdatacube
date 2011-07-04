#ifndef QDATACUBE_ABSTRACT_FORMATTER_H
#define QDATACUBE_ABSTRACT_FORMATTER_H

#include <QObject>
#include "qdatacube_export.h"
#include <QSize>
#include <QWidget>

class QAbstractItemModel;

namespace qdatacube {

class datacube_view_t;


/**
 * Subclasses of these provides summaries of sets of rows from the underlying model,
 * such as the sum or the average of some property.
 *
 * The property must be representable as a double, at least for now
 */
class QDATACUBE_EXPORT abstract_formatter_t : public QObject
{
  Q_OBJECT
  public:
    /**
     * @param underlying_model The model this summarize operates over
     * @param view the datacube view to use for fonts, palette etc.
     *
     * To install the formatter, use datacube_view->add_formatter()
     */
    abstract_formatter_t(QAbstractItemModel* underlying_model, datacube_view_t* view);

    /**
     * @return the accumulator, suitably formatted for output. E.g, units could be added, rounding
     * performed, average calculated
     */
    virtual QString format(QList<int> rows) const = 0;

    /**
     * @return short (3 letters or so) name of summary
     */
    virtual QString short_name() const = 0;

    /**
     * @return name for summary
     */
    virtual QString name() const = 0;

    /**
     * @return suggested cell size for this format.
     */
    QSize cell_size() const;
  Q_SIGNALS:
    /**
     * Emitted when the size of the cell of the formatter is changed
     */
    void cell_size_changed(QSize newsize);
  protected:

    /**
     * Set the size of this formatters cell.If this is the sole formatter, this will be the size of
     * the cell, if there is more than one the width of the cell with be the maximum with and the height
     * the sum of all the formatters' heights.
     * Calling this will cause cell_size_changed() to be emitted
     */
    void set_cell_size(QSize size);

    QAbstractItemModel* m_underlying_model;

    QSize m_cell_size;

    datacube_view_t* m_datacube_view;

};
} // end of namespace
#endif // ABSTRACT_SUMMARIZER_H
