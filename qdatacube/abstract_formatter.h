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
class AbstractFormatterPrivate;
class QDATACUBE_EXPORT abstract_formatter_t : public QObject
{
  Q_OBJECT
  public:
      enum UpdateType {
          CellSize = 0,
          UserType = 255
      };
    /**
     * @param underlying_model The model this summarize operates over
     * @param view the datacube view to use for fonts, palette etc.
     *
     * To install the formatter, use datacube_view->add_formatter()
     */
    abstract_formatter_t(QAbstractItemModel* underlying_model, datacube_view_t* view = 0);

    /**
     * @return the accumulator, suitably formatted for output. E.g, units could be added, rounding
     * performed, average calculated
     */
    virtual QString format(QList<int> rows) const = 0;

    /**
     * @return short (3 letters or so) name of summary
     */
    QString short_name() const;

    /**
     * @return name for summary
     */
    QString name() const;

    /**
     * @return suggested cell size for this format.
     */
    QSize cell_size() const;

    /**
     * @return the model underneath this formatter.
     */
    QAbstractItemModel* underlyingModel() const;

    /**
     * @return the datacube view that this formatter is formatting for.
     */
    datacube_view_t* datacubeView() const;

    /**
     * dtor
     */
    virtual ~abstract_formatter_t();
    /**
     * \override
     * for now, to catch font change events on the main widget.
     */
    virtual bool eventFilter(QObject* filter, QEvent* event );
  Q_SIGNALS:
    /**
     * Emitted when the size of the cell of the formatter is changed
     */
    void cell_size_changed(QSize newsize);

    /**
     * emitted when something in the formatter changed so that things needs to be recalculated
     */
    void formatter_changed();
  protected:

    /**
     * Set the size of this formatters cell.If this is the sole formatter, this will be the size of
     * the cell, if there is more than one the width of the cell with be the maximum with and the height
     * the sum of all the formatters' heights.
     * Calling this will cause cell_size_changed() to be emitted
     */
    void set_cell_size(QSize size);

    /**
     * Sets the short name to \param newShortName
     */
    void setShortName(const QString& newShortName);

    /**
     * sets the name to \param newName
     */
    void setName(const QString& newName);

    /**
     * Does a update of \param element
     * Default implementation does nothing.
     */
    virtual void update(abstract_formatter_t::UpdateType element);


    private:
        QScopedPointer<AbstractFormatterPrivate> d;

};
} // end of namespace
#endif // ABSTRACT_SUMMARIZER_H
