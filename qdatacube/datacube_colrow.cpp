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
     * Split the childindex *including* the empty ones
     * @param index
     * @param filter an abstract filter direviate, wrapped in a shared_ptr for convenience.
     */
    void split_including_empty(int index, std::tr1::shared_ptr<abstract_filter_t> filter);

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
};

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

datacube_colrow_t::datacube_colrow_t( const QAbstractItemModel* model, std::tr1::shared_ptr< qdatacube::abstract_filter_t > filter, const QList< int >& active
) :
    d(new secret_t(model, std::tr1::shared_ptr<abstract_filter_t>(filter)))
{
  d->sort_to_buckets(active);
  Q_ASSERT(d->buckets.size() == d->children.size());
}


void datacube_colrow_t::restrict(QList< int > set) {
  for (int i=0; i<d->buckets.size(); ++i) {
    d->buckets[i].clear();
  }
  d->sort_to_buckets(set);
  for(int i=0; i<d->children.size(); ++i) {
    datacube_colrow_t* child = d->children[i];
    if (child) {
      child->restrict(d->buckets[i]);
    }
  }

}

void datacube_colrow_t::remove(int index) {
  for (int i=0; i<d->buckets.size(); ++i) {
    if (d->buckets[i].contains(index)) {
      d->buckets[i].removeOne(index);
      if (datacube_colrow_t* child = d->children[i]) {
        child->remove(index);
      }
      break; // Optimization
    }
  }

}

int datacube_colrow_t::bucket_for_index(int index) const {
  int section =0;
  for (int i=0; i<d->buckets.size(); ++i) {
    if (d->buckets[i].contains(index)) {
      if (datacube_colrow_t* child = d->children[i]) {
        int rv = child->bucket_for_index(index);
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

void datacube_colrow_t::readd(int index) {
  int bucket_index = (*d->filter)(d->model, index);
  QList<int>& bucket = d->buckets[bucket_index];
  QList<int>::iterator it = std::lower_bound(bucket.begin(), bucket.end(), index);
  bucket.insert(it, index);
  if(datacube_colrow_t* child = d->children[bucket_index]) {
    child->readd(index);
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
  foreach (int index, list) {
    int bucket = (*filter)(model, index);
    buckets[bucket] << index;
  }
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

QList< int > datacube_colrow_t::indexes
(int section) const {
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

void datacube_colrow_t::split(int section, std::tr1::shared_ptr< abstract_filter_t > filter) {
  int index = d->index_for_section(section);
  d->split_including_empty(index, filter);
}

void datacube_colrow_t::split(std::tr1::shared_ptr< abstract_filter_t > filter) {
  for (int i=0; i<d->children.size(); ++i) {
    d->split_including_empty(i, filter);
  }
}

void datacube_colrow_t::split(abstract_filter_t* filter) {
  split(shared_ptr<abstract_filter_t>(filter));
}

void datacube_colrow_t::secret_t::split_including_empty(int index, std::tr1::shared_ptr< abstract_filter_t > filter) {
  datacube_colrow_t* newchild = new datacube_colrow_t(model, filter, buckets[index]);
  if (datacube_colrow_t* oldchild = children[index]) {
    for(int i=0; i<newchild->d->children.size(); ++i) {
      datacube_colrow_t*& grandchild = newchild->d->children[i];
      Q_ASSERT(grandchild == 0);
      grandchild = new datacube_colrow_t(model, oldchild->d->filter, newchild->d->buckets[i]);
    }
    delete oldchild;
  }
  children[index] = newchild;

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

qdatacube::datacube_colrow_t::~datacube_colrow_t() {
  // Only declared so that the QScopedPointer knows the destructor to secret_t
}

}

#include "datacube_colrow.moc"
