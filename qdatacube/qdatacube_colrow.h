/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#ifndef QDATACUBE_QDATACUBE_COLROW_H
#define QDATACUBE_QDATACUBE_COLROW_H

#include <QList>
#include <QObject>
#include <tr1/memory>
#include <QPair>

#include "qdatacube_export.h"

class QAbstractItemModel;

namespace qdatacube {
class abstract_filter_t;

/**
 * A row or column header in the recap table. Note that for recaps, there might
 * might be several row and column headers.
 * Note also the concept of index, which is a raw index, and section, which excludes
 * empty columns/rows. Index should not appear in the public interface.
 */
class QDATACUBE_EXPORT qdatacube_colrow_t : public QObject {
  Q_OBJECT
  public:

    /**
     * Construct top-level colrow_t
     * @param model the model the data is taken from
     * @param filter that defines the (top-level) columns. This takes ownership
     *        of filter, and deletes the filter when done.
     * @param active
     *   list of containers to limit to, will only show the containers that has their
     *   index from the model in this list
     */
    qdatacube_colrow_t( const QAbstractItemModel* model, abstract_filter_t* filter, const QList< int >& active);

    /**
     * @return the model for this row/col
     */
    const QAbstractItemModel* model() const {
      return m_model;
    }

    /**
     * @returns the number of headers, excluding the once without any containers,
     * recursively
     */
    int size() const;

    /**
     * @returns number of section in this header (non-recursively)
     */
    int sectionCount() const;

    /**
     * @returns a list of active headers, that is, those who are not empty
     * @param depth how many layers down to fetch the headers
     */
    QList<QPair<QString,int> > active_headers(int depth=0) const;

    /**
     * @returns the nth direct child
     */
    qdatacube_colrow_t* child(int section) const;

    /**
     * @returns the section descendant at the depth depth together with the correct section
     * the return value can then be used directly in e.g. split() or container_indexes();
     */
    QPair<qdatacube_colrow_t*,int> descendant_section(int depth, int section);

    /**
     * @returns max depth of children
     */
    int depth() const;

    /**
     * @returns container indexes for (sub) bucket
     *  recurses as needed
     */
    QList<int> container_indexes(int section) const;

    /**
     * @returns the bucket this container fits in. Recurses all the way to the bottom
     */
    int bucket_for_container(int container_index) const;

    /**
     * @returns container indexes for all containers under this
     * section (no recursion)
     */
    QList<int> all_container_indexes(int section) const;

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
     * Restrict colrow and descendants to this list.
     */
    void restrict(QList<int> set);

    /**
     * remove this container from set
     */
    void remove(int container_index);

    /**
     * readds a container with the specific index
     */
    void readd(int container_index);

  private:
    QList<QList<int> > m_buckets;
    QList<qdatacube_colrow_t*> m_children;
    const QAbstractItemModel* m_model;
    std::tr1::shared_ptr<abstract_filter_t> m_filter;

    /**
     * Reconstruct child qdatacube_colrow_t
     */
    qdatacube_colrow_t(const QAbstractItemModel* model,
                   const QList<int>& indexes,
                   std::tr1::shared_ptr<abstract_filter_t> filter);

    /**
     * Sort the list into buckets
     */
    void sort_to_buckets(const QList<int>& list);

    /**
     * Split the childindex *including* the empty ones
     */
    void split_including_empty(int index, std::tr1::shared_ptr<abstract_filter_t> filter);

    /**
     * @returns Span of current to depth
     */
    int fanthom_span(int maxdepth) const;

    /**
     * @returns the index for section
     * @param section
     */
    int index_for_section(int section) const;

};
}
#endif // QDATACUBE_QDATACUBE_COLROW_H
