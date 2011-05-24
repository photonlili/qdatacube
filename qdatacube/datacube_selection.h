#ifndef DATACUBE_SELECTION_H
#define DATACUBE_SELECTION_H

#include <QObject>
#include "qdatacube_export.h"

namespace qdatacube {

class datacube_view_t;


class datacube_t;


/**
 * Handles tracking of selection on a datacube
 * The selection is automatically sync'ed to the
 * datacube's underlying model.
 *
 *
 * Usually, there is no need to instantiate this class manually, as one is created automatically by
 * the datacube_view_t
 */
class QDATACUBE_EXPORT datacube_selection_t : public QObject {
  Q_OBJECT
  public:
    datacube_selection_t(qdatacube::datacube_t* datacube, datacube_view_t* view);

    enum selection_status_t {
      UNSELECTED = 0,
      PARTIALLY_SELECTED = 1,
      SELECTED = 2
    };

    /**
     * Clears selection
     */
    void clear();

    /**
     * Add elements to selection
     */
    void add_elements(QList<int> elements);

    /**
    * Add element to selection
    */
    void add_element(int element);

    /**
     * Add cell to selection
     */
    void add_cell(int row, int column);

    /**
     * Get selection status from (view) cell
     */
    selection_status_t selection_status(int row, int column) const;
  Q_SIGNALS:
    /**
     * Selection status has changed for cell
     **/
    void selection_status_changed(int row, int column);
  private Q_SLOTS:
    void reset();

  private:
    void datacube_adds_element_to_bucket(int row, int column, int element);
    void datacube_removes_element_to_bucket(int row, int column, int element);
    class secret_t;
    friend class datacube_t;
    QScopedPointer<secret_t> d;
};

}
#endif // DATACUBE_SELECTION_H
