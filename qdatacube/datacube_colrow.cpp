/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_colrow.h"
#include "abstract_filter.h"

#include <algorithm>

namespace qdatacube {

datacube_colrow_t::datacube_colrow_t( const QAbstractItemModel* model, abstract_filter_t* filter, const QList< int >& active) :
    m_buckets(),
    m_children(),
    m_model(model),
    m_filter(filter)
{
  for (int i=0; i<m_filter->categories(model).size(); ++i) {
    m_buckets << QList<int>();
    m_children << 0L;
  }
  sort_to_buckets(active);
  Q_ASSERT(m_buckets.size() == m_children.size());
}

datacube_colrow_t::datacube_colrow_t(const QAbstractItemModel* model,
                               const QList< int >& indexes,
                               std::tr1::shared_ptr< abstract_filter_t > filter) :
    m_buckets(),
    m_children(),
    m_model(model),
    m_filter(filter)
{
  for (int i=0; i<m_filter->categories(model).size(); ++i) {
    m_buckets << QList<int>();
    m_children << 0L;
  }
  sort_to_buckets(indexes);
  Q_ASSERT(m_buckets.size() == m_children.size());

}

void datacube_colrow_t::restrict(QList< int > set) {
  for (int i=0; i<m_buckets.size(); ++i) {
    m_buckets[i].clear();
  }
  sort_to_buckets(set);
  for(int i=0; i<m_children.size(); ++i) {
    datacube_colrow_t* child = m_children[i];
    if (child) {
      child->restrict(m_buckets[i]);
    }
  }

}

void datacube_colrow_t::remove(int container_index) {
  for (int i=0; i<m_buckets.size(); ++i) {
    if (m_buckets[i].contains(container_index)) {
      m_buckets[i].removeOne(container_index);
      if (datacube_colrow_t* child = m_children[i]) {
        child->remove(container_index);
      }
      break; // Optimization
    }
  }

}

int datacube_colrow_t::bucket_for_container(int container_index) const {
  int section =0;
  for (int i=0; i<m_buckets.size(); ++i) {
    if (m_buckets[i].contains(container_index)) {
      if (datacube_colrow_t* child = m_children[i]) {
        int rv = child->bucket_for_container(container_index);
        Q_ASSERT(rv>=0);
        return section+rv;
      } else {
        return section;
      }
    } else {
      if(datacube_colrow_t* child = m_children[i]) {
        section+=child->size();
      } else {
        if(!m_buckets[i].empty()) {
          section++;
        }
      }
    }
  }
  return -1;
}

void datacube_colrow_t::readd(int index) {
  int bucket_index = (*m_filter)(m_model, index);
  QList<int>& bucket = m_buckets[bucket_index];
  QList<int>::iterator it = std::lower_bound(bucket.begin(), bucket.end(), index);
  bucket.insert(it, index);
  if(datacube_colrow_t* child = m_children[bucket_index]) {
    child->readd(index);
  }
}

int datacube_colrow_t::size() const {
  int rv = 0;
  for (int i=0; i<m_buckets.size(); ++i) {
    if ( m_buckets[i].size() > 0 ) {
      rv += m_children[i] ? m_children[i]->size() : 1;
    }
  }
  return rv;
}

int datacube_colrow_t::sectionCount() const {
  int rv = 0;
  for (int i=0; i<m_buckets.size(); ++i) {
    if ( m_buckets[i].size() > 0 ) {
      ++rv;
    }
  }
  return rv;
}

QList< QPair<QString,int> > datacube_colrow_t::active_headers(int depth) const {
  QList<QPair<QString,int> > rv;
  const QList<QString> cat(m_filter->categories(m_model));
  for (int i=0; i<cat.size(); ++i) {
    if (!m_buckets[i].isEmpty()) {
      if (depth>0) {
        if (datacube_colrow_t* child = m_children[i]) {
          rv.append(child->active_headers(depth-1));
        } else {
          rv << QPair<QString,int>(QString(),1);
        }
      } else {
        rv << QPair<QString,int>(cat.at(i),m_children[i] ? m_children[i]->size() : 1);
      }
    }
  }
  return rv;
}

datacube_colrow_t* datacube_colrow_t::child(int section) const {
  int index = index_for_section(section);
  if (index>=0) {
    return m_children[index];
  }
  return 0L;
}

void datacube_colrow_t::sort_to_buckets(const QList< int >& list) {
  foreach (int index, list) {
    int bucket = (*m_filter)(m_model, index);
    m_buckets[bucket] << index;
  }
}

int datacube_colrow_t::span(int section) const {
  datacube_colrow_t* c = child(section);
  return c ? c->size() : 1;
}

int datacube_colrow_t::depth() const {
  int rv = 0;
  for (int i=0; i<m_children.size(); ++i) {
    if (m_children[i] && !m_buckets[i].isEmpty()) {
      rv = std::max(m_children[i]->depth(), rv);
    }
  }
  return rv+1; // Include self, hence +1
}

QList< int > datacube_colrow_t::container_indexes(int section) const {
  int i=0;
  int child = 0;
  while (true) {
    Q_ASSERT(i<=section);
    int sp = span(child);
    if (i+sp > section) {
      int index = index_for_section(child);
      if (datacube_colrow_t* c = m_children[index]) {
        return c->container_indexes(section-i);
      } else {
        return m_buckets[index];
      }
    }
    i+=sp;
    ++child;
  }
  Q_ASSERT(false);
  return QList<int>();

}

int datacube_colrow_t::index_for_section(int section) const {
  // Skip empty buckets
  for (int i=0; i<m_buckets.size();++i) {
    if (m_buckets[i].isEmpty()) {
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
  int index = index_for_section(section);
  split_including_empty(index, filter);
}

void datacube_colrow_t::split(std::tr1::shared_ptr< abstract_filter_t > filter) {
  for (int i=0; i<m_children.size(); ++i) {
    split_including_empty(i, filter);
  }
}

void datacube_colrow_t::split_including_empty(int index, std::tr1::shared_ptr< abstract_filter_t > filter) {
  datacube_colrow_t* newchild = new datacube_colrow_t(m_model, m_buckets[index], filter);
  if (datacube_colrow_t* oldchild = m_children[index]) {
    for(int i=0; i<newchild->m_children.size(); ++i) {
      datacube_colrow_t*& grandchild = newchild->m_children[i];
      Q_ASSERT(grandchild == 0);
      grandchild = new datacube_colrow_t(m_model, newchild->m_buckets[i], oldchild->m_filter);
    }
    delete oldchild;
  }
  m_children[index] = newchild;

}

QPair<datacube_colrow_t*,int> datacube_colrow_t::descendant_section(int depth, int section) {
  if (depth == 0) {
    return QPair<datacube_colrow_t*,int>(this,section);
  } else {
    int i = 0;
    for (int c = 0;c<m_children.size();++c) {
      if (datacube_colrow_t* chld = m_children[c]) {
        int s = chld->fanthom_span(depth-1);
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

int datacube_colrow_t::fanthom_span(int maxdepth) const {
  if (maxdepth == 0) {
    return m_buckets.count() - m_buckets.count(QList<int>());
  }
  int rv = 0;
  foreach(datacube_colrow_t* child, m_children) {
    if (child) {
      rv += child->fanthom_span(maxdepth-1);
    } else {
      rv += 1;
    }
  }
  return rv;
}

QList< int > datacube_colrow_t::all_container_indexes(int section) const {
  int index = index_for_section(section);
  return m_buckets[index];
}
}

#include "datacube_colrow.moc"
