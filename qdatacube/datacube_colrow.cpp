/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_colrow.h"
#include "abstract_filter.h"

#include <algorithm>

using std::tr1::shared_ptr;

namespace qdatacube {

class datacube_colrow_t::secret_t {
  public:
    QList<QList<int> > buckets;
    QList<datacube_colrow_t*> children;
    const QAbstractItemModel* model;
    std::tr1::shared_ptr<abstract_filter_t> filter;

    secret_t(const QAbstractItemModel* model,
             std::tr1::shared_ptr<abstract_filter_t> filter);

    /**
     * Sort the list into buckets
     */
    void sort_to_buckets(const QList<int>& list);

    /**
     * Clear buckets recursively
     */
    void recursive_clear_buckets();

    /**
     * @returns Span of current to depth
     */
    int span(int maxdepth) const;

    /**
     * @returns the index for section
     * @param section
     */
    int index_for_section(int section) const;

    /**
     * @return the nth direct child
     */
    datacube_colrow_t* child(int section) const;

    /**
     * span of current to maxdepth
     */
    int fanthom_span(int maxdepth) const;

    /**
     * Create a deep copy of child with active elements
     */
    datacube_colrow_t* deep_copy(QList<int> active);
};

datacube_colrow_t* datacube_colrow_t::secret_t::deep_copy(QList< int > active) {
  datacube_colrow_t* rv = new datacube_colrow_t(model, filter, active);
  for (int bucketno=0; bucketno<children.size(); ++bucketno) {
    if (datacube_colrow_t* child = children[bucketno]) {
      rv->set_child(bucketno, child->deep_copy(rv->bucket_contents(bucketno)));
    }
  }
  return rv;
}

datacube_colrow_t::secret_t::secret_t(const QAbstractItemModel* model,
                                      std::tr1::shared_ptr< abstract_filter_t > filter) :
    buckets(),
    children(),
    model(model),
    filter(filter)
{
  for (int i=0; i<filter->categories(model).size(); ++i) {
    buckets << QList<int>();
    children << 0L;
  }

}

datacube_colrow_t* datacube_colrow_t::secret_t::child(int section) const {
  int index = index_for_section(section);
  if (index>=0) {
    return children[index];
  }
  return 0L;

}

void datacube_colrow_t::secret_t::recursive_clear_buckets() {
  for (int bucketno=0; bucketno<buckets.size(); ++bucketno) {
    children[bucketno]->d->recursive_clear_buckets();
    buckets[bucketno].clear();
  }
}

datacube_colrow_t::datacube_colrow_t( const QAbstractItemModel* model, std::tr1::shared_ptr< qdatacube::abstract_filter_t > filter, const QList< int >& active) :
    d(new secret_t(model, std::tr1::shared_ptr<abstract_filter_t>(filter)))
{
  d->sort_to_buckets(active);
  Q_ASSERT(!filter->categories(model).isEmpty());
  Q_ASSERT(d->buckets.size() == d->children.size());
  connect(filter.get(), SIGNAL(category_added(int)), SLOT(filter_catogory_added(int)));
  connect(filter.get(), SIGNAL(category_removed(int)), SLOT(filter_catogory_removed(int)));
}

int datacube_colrow_t::remove(int index) {
  int section = 0;
  for (int i=0; i<d->buckets.size(); ++i) {
    if (d->buckets[i].contains(index)) {
      d->buckets[i].removeOne(index);
      if (datacube_colrow_t* child = d->children[i]) {
        section += child->remove(index);
      }
      return section;
    } else {
      if(datacube_colrow_t* child = d->children[i]) {
        section+=child->size();
      } else {
        if(!d->buckets[i].empty()) {
          section++;
        }
      }
    }
  }
  return -1;

}

int datacube_colrow_t::section_for_index(int index) const {
  int section =0;
  for (int i=0; i<d->buckets.size(); ++i) {
    if (i == (*d->filter)(d->model, index)) {
      if (datacube_colrow_t* child = d->children[i]) {
        int rv = child->section_for_index(index);
        Q_ASSERT(rv>=0);
        return section+rv;
      } else {
        return section;
      }
    } else {
      if(datacube_colrow_t* child = d->children[i]) {
        section+=child->size();
      } else {
        if(!d->buckets[i].empty()) {
          section++;
        }
      }
    }
  }
  return -1;
}

int datacube_colrow_t::section_for_index_internal(int index) {
  int section =0;
  for (int i=0; i<d->buckets.size(); ++i) {
    if (d->buckets[i].contains(index)) {
      if (datacube_colrow_t* child = d->children[i]) {
        int rv = child->section_for_index(index);
        Q_ASSERT(rv>=0);
        return section+rv;
      } else {
        return section;
      }
    } else {
      if(datacube_colrow_t* child = d->children[i]) {
        section+=child->size();
      } else {
        if(!d->buckets[i].empty()) {
          section++;
        }
      }
    }
  }
  return -1;

}

QList<int> datacube_colrow_t::sibling_indexes(int index) const {
  const int bucketno = (*d->filter)(d->model, index);
  if (const datacube_colrow_t* child = d->children[bucketno]) {
    return child->sibling_indexes(index);
  } else {
    return d->buckets[bucketno];
  }
}

void datacube_colrow_t::add(int index) {
  int bucket_index = (*d->filter)(d->model, index);
  QList<int>& bucket = d->buckets[bucket_index];
  QList<int>::iterator it = std::lower_bound(bucket.begin(), bucket.end(), index);
  bucket.insert(it, index);
  if(datacube_colrow_t* child = d->children[bucket_index]) {
    child->add(index);
  }
}

int datacube_colrow_t::size() const {
  int rv = 0;
  for (int i=0; i<d->buckets.size(); ++i) {
    if ( d->buckets[i].size() > 0 ) {
      rv += d->children[i] ? d->children[i]->size() : 1;
    }
  }
  return rv;
}

int datacube_colrow_t::sectionCount() const {
  int rv = 0;
  for (int i=0; i<d->buckets.size(); ++i) {
    if ( d->buckets[i].size() > 0 ) {
      ++rv;
    }
  }
  return rv;
}

int datacube_colrow_t::bucket_count() const {
  return d->buckets.size();
}

QList< QPair<QString,int> > datacube_colrow_t::active_headers(int depth) const {
  QList<QPair<QString,int> > rv;
  const QList<QString> cat(d->filter->categories(d->model));
  for (int i=0; i<cat.size(); ++i) {
    if (!d->buckets[i].isEmpty()) {
      if (depth>0) {
        if (datacube_colrow_t* child = d->children[i]) {
          rv.append(child->active_headers(depth-1));
        } else {
          rv << QPair<QString,int>(QString(),1);
        }
      } else {
        rv << QPair<QString,int>(cat.at(i),d->children[i] ? d->children[i]->size() : 1);
      }
    }
  }
  return rv;
}

datacube_colrow_t* datacube_colrow_t::child(int section) const {
  return d->child(section);
}

void datacube_colrow_t::secret_t::sort_to_buckets(const QList< int >& list) {
  Q_FOREACH(int index, list) {
    int bucket = (*filter)(model, index);
    buckets[bucket] << index;
  }
}

datacube_colrow_t* datacube_colrow_t::child_for_bucket(int bucketno) const {
  return d->children.at(bucketno);
}

int datacube_colrow_t::span(int section) const {
  datacube_colrow_t* c = d->child(section);
  return c ? c->size() : 1;
}

int datacube_colrow_t::depth() const {
  int rv = 0;
  for (int i=0; i<d->children.size(); ++i) {
    if (d->children[i] && !d->buckets[i].isEmpty()) {
      rv = std::max(d->children[i]->depth(), rv);
    }
  }
  return rv+1; // Include self, hence +1
}

QList< int > datacube_colrow_t::indexes(int section) const {
  int i=0;
  int child = 0;
  while (true) {
    Q_ASSERT(i<=section);
    int sp = span(child);
    if (i+sp > section) {
      int index = d->index_for_section(child);
      if (datacube_colrow_t* c = d->children[index]) {
        return c->indexes(section-i);
      } else {
        return d->buckets[index];
      }
    }
    i+=sp;
    ++child;
  }
  Q_ASSERT(false);
  return QList<int>();

}

QList< int > datacube_colrow_t::bucket_contents(int bucketno) const {
  return d->buckets.at(bucketno);
}

bool datacube_colrow_t::bucket_empty(int bucketno) const {
  return d->buckets.at(bucketno).empty();
}

int datacube_colrow_t::secret_t::index_for_section(int section) const {
  // Skip empty buckets
  for (int i=0; i<buckets.size();++i) {
    if (buckets[i].isEmpty()) {
      continue;
    }
    if (section == 0) {
      return i;
    }
    --section;
  }
  Q_ASSERT(false);
  return -1;
}

QPair<datacube_colrow_t*,int> datacube_colrow_t::descendant_section(int depth, int section) {
  if (depth == 0) {
    return QPair<datacube_colrow_t*,int>(this,section);
  } else {
    int i = 0;
    for (int c = 0;c<d->children.size();++c) {
      if (datacube_colrow_t* chld = d->children[c]) {
        int s = chld->d->fanthom_span(depth-1);
        if (s+i>section) {
          return chld->descendant_section(depth-1, section-i);
        } else {
          i += s;
        }
      } else if (i+1<=section) {
        ++i;
      } else {
        return QPair<datacube_colrow_t*,int>(0L,-1); // Failed
      }
    }
    return QPair<datacube_colrow_t*,int>(0L,-1); // Failed
  }
}

int datacube_colrow_t::secret_t::fanthom_span(int maxdepth) const {
  if (maxdepth == 0) {
    return buckets.count() - buckets.count(QList<int>());
  }
  int rv = 0;
  foreach(datacube_colrow_t* child, children) {
    if (child) {
      rv += child->d->fanthom_span(maxdepth-1);
    } else {
      rv += 1;
    }
  }
  return rv;
}

QList< int > datacube_colrow_t::all_indexes
(int section) const {
  int index = d->index_for_section(section);
  return d->buckets[index];
}

datacube_colrow_t::~datacube_colrow_t() {
  // Only declared so that the QScopedPointer knows the destructor to secret_t
}


void datacube_colrow_t::adjust_after_remove(int index) {
  for (int bucketno = 0; bucketno<d->buckets.size(); ++bucketno) {
    bool any_adjusted = false;
    QList<int>& bucket = d->buckets[bucketno];
    for (int i=0; i<bucket.size(); ++i) {
      int& idx =bucket[i];
      Q_ASSERT(idx!=index); // index assumed already removed
      if (idx>index) {
        --idx;
        any_adjusted = true;
      }
    }
    if (any_adjusted) {
      if (datacube_colrow_t* child = d->children[bucketno]) {
        child->adjust_after_remove(index);
      }
    }
  }
}

void datacube_colrow_t::adjust_before_add(int cutoff, int amount) {
  for (int bucketno = 0; bucketno<d->buckets.size(); ++bucketno) {
    bool any_adjusted = false;
    QList<int>& bucket = d->buckets[bucketno];
    for (int i=0; i<bucket.size(); ++i) {
      int& idx =bucket[i];
      if (idx>=cutoff) {
        idx+=amount;
        any_adjusted = true;
      }
      Q_ASSERT((*d->filter)(d->model,idx) == bucketno);
    }
    if (any_adjusted) {
      if (datacube_colrow_t* child = d->children[bucketno]) {
        child->adjust_before_add(cutoff, amount);
      }
    }
  }

}

void datacube_colrow_t::set_child(int bucketno, datacube_colrow_t* child) {
  datacube_colrow_t*& oldchild = d->children[bucketno];
  delete oldchild;
  oldchild = child;
}

qdatacube::datacube_colrow_t* datacube_colrow_t::deep_copy(QList< int > rows) {
  return d->deep_copy(rows);
}

std::tr1::shared_ptr< qdatacube::abstract_filter_t > datacube_colrow_t::filter() const {
  return d->filter;
}

QList< std::tr1::shared_ptr< abstract_filter_t > > datacube_colrow_t::filters_for_section(int section) const {
  QList< std::tr1::shared_ptr< abstract_filter_t > > rv;
  rv << d->filter;
  int i=0;
  int child = 0;
  while (true) {
    Q_ASSERT(i<=section);
    int sp = span(child);
    if (i+sp > section) {
      int index = d->index_for_section(child);
      if (datacube_colrow_t* c = d->children[index]) {
        rv << c->filters_for_section(section-i);
        return rv;
      } else {
        return rv;
      }
    }
    i+=sp;
    ++child;
  }
  Q_ASSERT(false);
}

void datacube_colrow_t::filter_catogory_added(int index) {
  d->buckets.insert(index, QList<int>());
  //Copy the old child here, but with no actives
  datacube_colrow_t* template_child = d->children.at(0);
  d->children.insert(index, template_child ? template_child->deep_copy(QList<int>()) : 0L);
}

void datacube_colrow_t::filter_catogory_removed(int index) {
  Q_ASSERT(d->buckets.at(index).isEmpty());
  d->buckets.removeAt(index);
  d->children.removeAt(index);
}


}


#include "datacube_colrow.moc"
