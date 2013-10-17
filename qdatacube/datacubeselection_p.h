#ifndef QDATACUBE_DATACUBESELECTION_P
#define QDATACUBE_DATACUBESELECTION_P
#include <QObject>
#include <QItemSelectionModel>

class QItemSelectionModel;
namespace qdatacube {

class DatacubeSelection;

class Datacube;

class DatacubeSelectionPrivate : public QObject {
    Q_OBJECT
    public:
        DatacubeSelectionPrivate(DatacubeSelection* datacubeselection);
        DatacubeSelection* q;
        Datacube* datacube;
        QVector<int> cells;
        QSet<int> selected_elements;
        QItemSelectionModel* synchronized_selection_model;
        bool ignore_synchronized;
        int nrows;
        int ncolumns;

        int& cell(int row, int column);

        void dump();

        void select_on_synchronized(QList<int> elements);
        void deselect_on_synchronized(QList<int> elements);
        void clear_synchronized();
        QItemSelection map_to_synchronized(QList<int> elements);
        QList<int> elements_from_selection(QItemSelection selection);
        void datacube_adds_element_to_bucket(int row, int column, int element);
        void datacube_removes_element_from_bucket(int row, int column, int element);
        void datacube_deletes_elements(int start, int end);
        void datacube_inserts_elements(int start, int end);
    public Q_SLOTS:
        void reset();


};

} // namespace

#endif // QDATACUBE_DATACUBESELECTION_P
