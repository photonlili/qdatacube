#ifndef DATACUBE_SELECTION_MODEL_H
#define DATACUBE_SELECTION_MODEL_H

#include <QItemSelectionModel>

#include "qdatacube_export.h"

namespace qdatacube {

class datacube_selection_model_private_t;

class QDATACUBE_EXPORT datacube_selection_model_t : public QItemSelectionModel {
  Q_OBJECT
  public:
    datacube_selection_model_t(QAbstractItemModel* model);
    virtual ~datacube_selection_model_t();

    /**
     * @returns true if partially (including fully) selected
     */
    bool partially_selected(const QModelIndex& index) const;
  public Q_SLOTS:
    /**
     * Elements selected. This slot must be called whenever some elements have been selected.
     */
    void select_elements(const QList<int>& elements);

    /**
     * Elements selected. This slot must be called whenever some elements have been deselected.
     */
    void deselect_elements(const QList<int>& elements);

    /**
     * @overrides.
     * Overridden to update internal state re partial selections.
     */
    virtual void select(const QModelIndex& index, SelectionFlags command);

    /**
     * @overrides.
     * Overridden to update internal state re partial selections.
     */
    virtual void select(const QItemSelection& selection, SelectionFlags command);
  private Q_SLOTS:
    /**
     * Connect to model to update internal state re partial selections
     */
    void slot_insert_rows(const QModelIndex& parent, int begin, int end);
    /**
     * Connect to model to update internal state re partial selections
     */
    void slot_insert_columns(const QModelIndex&, int end, int begin);

    /**
     * Connect to model to update internal state re partial selections
     */
    void slot_remove_rows(const QModelIndex& parent, int begin, int end);

    /**
     * Connect to model to update internal state re partial selections
     */
    void slot_remove_columns(const QModelIndex& parent, int begin, int end);
  Q_SIGNALS:
    void partial_selection_changed(const QItemSelection& selection, const QItemSelection& deselection);

  private:
    class secret_t;
    QScopedPointer<secret_t> d;
    void clear_implementation();
};
}
#endif // DATACUBE_SELECTION_MODEL_H
