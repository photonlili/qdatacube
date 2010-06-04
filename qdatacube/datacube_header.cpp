/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "datacube_header.h"

#include <QPaintEvent>
#include <QPainter>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QTableView>
#include <QLayout>

#include "datacube.h"
#include "datacube_header_model.h"
#include "datacube_model.h"

namespace qdatacube {

class datacube_header_t::secret_t {
  public:
    QSize size;
    QList<QHeaderView*> headers;
    bool lock;
    QLayout* layout;
    /**
     * Helper function to partition the size changed (oldsize-size) into
     * span integer pieces. This function will fetch the i'th piece.
     * The essential property of this function is that given a larger number
     * of resizes, the size changed should be evenly distributed across the
     * pieces
     */
    int getnewlhsize(int oldsize, int size, int i, int span);

    secret_t();
};

datacube_header_t::secret_t::secret_t() :
    size(),
    headers(),
    lock(false),
    layout(0L)
{

}


datacube_header_t::datacube_header_t(Qt::Orientation orientation, QWidget* parent):
    QHeaderView(orientation, parent),
    d(new secret_t)
{
  if (orientation == Qt::Horizontal) {
    d->layout = new QVBoxLayout(this);
  } else {
    d->layout = new QHBoxLayout(this);
  }
  d->layout->setSpacing(0);
  d->layout->setMargin(0);
  connect(this, SIGNAL(sectionResized(int,int,int)), this, SLOT(slotSectionResized(int,int,int)));
  QAbstractScrollArea* scroll_area =  dynamic_cast<QAbstractScrollArea*>(parent);
  connect(orientation == Qt::Horizontal ? scroll_area->horizontalScrollBar() : scroll_area->verticalScrollBar(),
          SIGNAL(valueChanged(int)), this, SLOT(setAllOffset(int)));
}

QSize datacube_header_t::sizeHint() const {
  QSize rv;
  if (orientation() == Qt::Horizontal) {
    foreach (QHeaderView* headerview, d->headers) {
      QSize size = headerview->sizeHint();
      rv.setHeight(std::max(rv.height()+size.height(),20));
    }
    if (!d->headers.isEmpty()) {
      rv.setWidth(std::max(d->headers.last()->width(),100));
    }
  } else {
    foreach (QHeaderView* headerview, d->headers) {
      QSize size = headerview->sizeHint();
      rv.setWidth(std::max(rv.width()+size.width(),10));
    }
    if (!d->headers.isEmpty()) {
      rv.setHeight(std::max(d->headers.last()->height(),200));
    }
  }
  return rv;
}

void datacube_header_t::setModel(QAbstractItemModel* model) {
  QHeaderView::setModel(model);
}

void datacube_header_t::setAllOffset(int value) {
  QAbstractScrollArea* scroll_area =  dynamic_cast<QAbstractScrollArea*>(parent());
  if (!scroll_area) {
    return;
  }
  QScrollBar* bar = orientation() == Qt::Horizontal ? scroll_area->horizontalScrollBar() : scroll_area->verticalScrollBar();
  Q_ASSERT(bar);
  int max = bar->maximum();
  if (value != max) {
    foreach (QHeaderView* headerview, d->headers) {
      headerview->setOffsetToSectionPosition(value);
    }
  } else if (max>0) {
    foreach (QHeaderView* headerview, d->headers) {
      headerview->setOffsetToLastSection();
    }
  }
}

void datacube_header_t::updateSizes() {
  if (d->headers.empty()) {
    return;
  }
  QHeaderView* lastheader = d->headers.last();
  for (int index = 0; index<d->headers.size(); ++index) {
    QHeaderView* headerview = d->headers[index];
    if (headerview != lastheader) {
      int lhindex = 0;
      for (int j=0;j<d->headers[index]->count();++j) {
        int span = headerview->model()->headerData(j, orientation(), Qt::UserRole+1).toInt();
        int size = 0;
        for (int k=0; k<span; ++k) {
          size += lastheader->sectionSize(lhindex++);
        }
        headerview->resizeSection(j, size);
        d->headers[index]->resizeSection(j,size);
      }
    }
    ++index;
  }

}

void datacube_header_t::setSectionSize(int section,int oldsize, int size) {
  if (d->lock) {
    return;
  }
  d->lock = true;
  if (QHeaderView* header = qobject_cast<QHeaderView*>(sender())) {
    int index = d->headers.indexOf(header);
    if (index < d->headers.size()-1) {
      int lhindex = 0;
      for (int i=0;i<section;++i) {
        int span = header->model()->headerData(i, orientation(), Qt::UserRole+1).toInt();
        lhindex+=span;
      }
      int span = header->model()->headerData(section, orientation(), Qt::UserRole+1).toInt();
      for (int i=0; i<span;++i) {
        int oldlhsize = d->headers.last()->sectionSize(i+lhindex);
        int newlhsize = oldlhsize + d->getnewlhsize(oldsize, size, i, span );

        d->headers.last()->resizeSection(lhindex+i, newlhsize);
        resizeSection(lhindex+i, newlhsize);
      }
    } else {
      resizeSection(section, size);
    }
  }
  updateSizes();
  d->lock = false;
}

int datacube_header_t::secret_t::getnewlhsize(int oldsize, int size, int i, int span) {
  int sizechange = size-oldsize;
  int rv;
  if (sizechange > 0) {
    int offset = i + oldsize % span; // Using offset instead of i dsitributes the sizechange across the pieces
    rv = sizechange*(offset+1)/span - sizechange*offset/span;
  } else {
    int offset = i + size % span; // Using offset instead of i dsitributes the sizechange across the pieces
    rv = -((-sizechange*(offset+1))/span - (-sizechange)*offset/span);
  }
  return rv;
}

void datacube_header_t::slotSectionResized(int section, int /*oldsize*/, int size) {
  d->headers.last()->resizeSection(section, size);
}

void datacube_header_t::reset() {
  const Qt::Orientation orientation = this->orientation();
  foreach(QHeaderView* header, d->headers) {
    QAbstractItemModel* header_model = header->model();
    delete header;
    delete header_model;
  }
  d->headers.clear();
  if (datacube_model_t* datacube_model = qobject_cast<datacube_model_t*>(model())) {
    datacube_t* datacube = datacube_model->datacube();
    for (int i=0, iend = datacube->depth(orientation); i<iend; ++i) {

      QHeaderView* headerview = new QHeaderView(orientation, this);
      d->layout->addWidget(headerview);
      connect(headerview, SIGNAL(sectionResized(int,int,int)), SLOT(setSectionSize(int,int,int)));
      d->headers << headerview;
      if (datacube_model) {
        datacube_header_model_t* rhm = new datacube_header_model_t(datacube_model, orientation, i);
        connect(rhm,SIGNAL(modelReset()),this,SLOT(reset()));
        headerview->setModel(rhm);
      }

    }
  } else {
    return;
  }
  // Set minimum sizes for headers
  QHeaderView* lastheader = d->headers.last();
  Q_ASSERT(lastheader);
  for (int section=0; section < lastheader->count(); ++section) {
    setMinimumSectionSize(std::max(lastheader->minimumSectionSize(),20));
  }
  foreach (QHeaderView* header, d->headers) {
    if (header != lastheader) {
      for (int i=0; i<header->count(); ++i) {
        int span = header->model()->headerData(i, orientation, Qt::UserRole+1).toInt();
        int minsize = qMax(qMax(span*lastheader->minimumSectionSize(), header->minimumSectionSize()),30);
        int key = d->headers.indexOf(header);
        header->setMinimumSectionSize(minsize);
        d->headers.value(key)->setMinimumSectionSize(minsize);

      }
    }
  }
  d->lock = true;
  for (int i=0; i<count();++i) {
    d->headers.last()->resizeSection(i,sectionSize(i));
    lastheader->resizeSection(i, sectionSize(i));
    d->headers.last()->showSection(i);
  }

  updateSizes();
  d->lock = false;
  d->layout->update();
  if(size().width() ==0){
    resize(20,size().height());
  }
  if(size().height() == 0){
    resize(size().width(),40);
  }
  QHeaderView::reset();

}

void datacube_header_t::paintEvent(QPaintEvent* e) {
  QWidget::paintEvent(e);
}

int datacube_header_t::sizeHintForColumn(int ) const {
  return -1; // Perhaps TODO, but this avoid unforunate recursion
}

int datacube_header_t::sizeHintForRow(int ) const {
  return -1; // Perhaps TODO, but this avoid unforunate recursion
}
}
#include "datacube_header.moc"
