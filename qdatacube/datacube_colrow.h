/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef Q_DATACUBE_DATACUBE_COLROW_H
#define Q_DATACUBE_DATACUBE_COLROW_H

#include <QList>
#include <QObject>
#include <tr1/memory>
#include <QPair>

#include "qdatacube_export.h"

class QAbstractItemModel;

namespace qdatacube {
class abstract_filter_t;

/**
 * A row or column header in the datacube table. Note that for datacubes, there might
 * might be several row and column headers.
 * Note also the concept of index, which is a raw index, and section, which excludes
 * empty columns/rows. Index should not appear in the public interface. Not to be confused with the indexes
 * which are row indexes in the underlying model.
 */
class QDATACUBE_EXPORT datacube_colrow_t : public QObject {
  Q_OBJECT
  public:

    /**
     * Construct top-level colrow_t
     * @param model the model the data is taken from
     * @param filter that defines the (top-level) columns. This takes ownership
     *        of filter, and deletes the filter when done.
     * @param active
     *   list of indexes to limit to, will only show the elements that has their (FIXME: Ticket #115)
     *   index from the model in this list
     */
    datacube_colrow_t(const QAbstractItemModel* model, std::tr1::shared_ptr<abstract_filter_t> filter, const QList< int >& active);

    /**
     * destructor
     */
    virtual ~datacube_colrow_t();

    /**
     * @return the underlying model for this row/col
     */
    const QAbstractItemModel* model() const ;

    /**
     * @returns the number of headers, excluding the once without any elements,
     * recursively (that is, including subheaders)
     */
    int size() const;

    /**
     * @returns number of section in this header (non-recursively)
     */
    int sectionCount() const;

    /**
     * @returns a list of active headers, that is, those who are not empty. The second
     * @param depth how many layers down to fetch the headers
     */
    QList<QPair<QString,int> > active_headers(int depth=0) const;

    /**
     * @returns the nth direct child
     */
    datacube_colrow_t* child(int section) const;

    /**
     * @returns the section descendant at the depth depth together with the correct section
     * the return value can then be used directly in e.g. split() or indexes();
     */
    QPair<datacube_colrow_t*,int> descendant_section(int depth, int section);

    /**
     * @returns max depth of children
     */
    int depth() const;

    /**
     * @returns container indexes for (sub) bucket
     *  recurses as needed
     */
    QList<int> indexes(int section) const;

    /**
     * @returns the bucket this container fits in. Recurses all the way to the bottom
     */
    int bucket_for_index(int index) const;

    /**
     * @returns container indexes for all elements under this
     * section (no recursion)
     */
    QList<int> all_indexes(int section) const;

    /**
     * @returns Span of section, recursively to the bottom
     */
    int span(int section) const;

    /**
     * Split the nth child according to filter
     * @param child/section to split
     * @param filter filter to use
     */
    void split(int section, std::tr1::shared_ptr<abstract_filter_t> filter);

    /**
     * Split all children according to filter. Note that this includes non-showing children
     * @param child/section to split
     */
    void split(std::tr1::shared_ptr<abstract_filter_t> filter);

    /**
     * Convience method for split. As above, but claims ownership of filter, which will be deleted at some point
     */
    void split(abstract_filter_t* filter);

    /**
     * Restrict colrow and descendants to this list.
     */
    void restrict(QList<int> set);

    /**
     * remove index set
     */
    void remove(int index);

    /**
     * readds a container with the specific index
     */
    void readd(int container_index);

  private:
    class secret_t;
    QScopedPointer<secret_t> d;

};
}
#endif // Q_DATACUBE_DATACUBE_COLROW_H
