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
class datacube_colrow_t : public QObject {
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
     * @returns the number of headers, excluding the ones without any elements,
     * recursively (that is, including subheaders)
     */
    int size() const;

    /**
     * @returns number of section in this header (non-recursively), excluding ones without any elements
     */
    int sectionCount() const;

    /**
     * @return number of buckets in colrow (that woudl include the empty ones, non-recursively)
     */
    int bucket_count() const;

    /**
     * @returns a list of active headers, that is, those who are not empty. The second
     * @param depth how many layers down to fetch the headers
     */
    QList<QPair<QString,int> > active_headers(int depth=0) const;

    /**
     * @returns the nth direct child or null if there is no such child
     */
    datacube_colrow_t* child(int section) const;

    /**
     * @return the child of bucket no, or null if there is no such child
     */
    datacube_colrow_t* child_for_bucket(int bucketno) const;

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
     * @returns indexes for section
     *  recurses as needed
     */
    QList<int> indexes(int section) const;

    /**
     * @return contents of bucket (that is, all indexes under this bucket, childs included)
     */
    QList<int> bucket_contents(int bucketno) const;

    /**
     * @return true if bucket is empty
     */
    bool bucket_empty(int bucketno) const;

    /**
     * @returns the section this container fits in. Recurses all the way to the bottom. Note that if index is not
     * currently in the datacube (due to a filter), the section returned is the section it will be in if inserted now.
     */
    int section_for_index(int index) const;

    /**
     * @returns the section this underlying index resides in. Recurses all the way to the bottom. Note that if index is not
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

    /**
     * Set the child for bucket, deleting the old one.
     */
    void set_child(int bucketno, datacube_colrow_t* child);

    /**
     * Create a deep copy with rows
     */
    datacube_colrow_t* deep_copy(QList< int > rows);

    /**
     * @return filter
     */
    std::tr1::shared_ptr<abstract_filter_t> filter() const;

    /**
     * @return all filters that apply to section, recursively
     */
    QList<std::tr1::shared_ptr<abstract_filter_t> > filters_for_section(int section) const;

    /**
     * Return the bucketno the section belongs in. (Note that several sections might belong in the same bucketno)
     */
    int section_for_bucketno(int bucketno);


  private Q_SLOTS:
    void filter_catogory_added(int index);
    void filter_catogory_removed(int index);
  private:
    class secret_t;
    QScopedPointer<secret_t> d;

};
}
#endif // Q_DATACUBE_DATACUBE_COLROW_H
