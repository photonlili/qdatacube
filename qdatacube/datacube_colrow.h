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
     * @returns indexes for (sub) bucket
     *  recurses as needed
     */
    QList<int> indexes(int section) const;

    /**
     * @returns the section this container fits in. Recurses all the way to the bottom. Note that if index is not
     * currently in the datacube (due to a filter), the section returned is the section it will be in if inserted now.
     */
    int section_for_index(int index) const;

    /**
     * @returns the section this container resides in. Recurses all the way to the bottom. Note that if index is not
     * currently in the datacube (due to a filter), -1 is returned.
     */
    int section_for_index_internal(int index);

    /**
     * @returns all indexes in the same (sub)category as the supplied index. Recurses as needed.
     */
    QList<int> sibling_indexes(int index) const;

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
     * remove index set, recursively
     * @return section where the index was removed from
     */
    int remove(int index);

    /**
     * readds a container with the specific index
     */
    void add(int index);

    /**
     * Compresses indexes after index has been removed. Used internally to adjust after the underlying model has removed
     * rows; users would probably not need to call this function.
     * The function assumes that the index has already been remove() from the colrow.
     */
    void adjust_after_remove(int index);

    /**
     * Adjust indexes before new indexes are added. Used internally to adjust before the underlying model insert new
     * rows; users would probably not need to call this function.
     * The function handles insertion of a range.
     * @param cutoff the smallest index that needs to be adjusted (i.e., the first index after the to-be-inserted range)
     * @param amoutn the number of indexes that will be inserted.
     */
    void adjust_before_add(int cutoff, int amount);
  Q_SIGNALS:
    // These signals are emitted after calling split and friends
    void sections_about_to_be_removed(int section, int count);
    void sections_about_to_be_inserted(int section, int count);
    void sections_removed(int section, int count);
    void sections_inserted(int section, int count);
    void sections_changed(int section, int count);
  private:
    /**
     * Split the childindex *including* the empty ones
     * @param index
     * @param filter an abstract filter direviate, wrapped in a shared_ptr for convenience.
     */
    void split_including_empty(int bucketno, int section, std::tr1::shared_ptr< qdatacube::abstract_filter_t > filter);
    class secret_t;
    QScopedPointer<secret_t> d;

};
}
#endif // Q_DATACUBE_DATACUBE_COLROW_H
