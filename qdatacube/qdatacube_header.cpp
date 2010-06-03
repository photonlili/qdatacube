/*
 Author: Ange Optimization <esben@ange.dk>  (C) Ange Optimization ApS 2009

 Copyright: See COPYING file that comes with this distribution

*/

#include "qdatacube_header.h"

#include <QPaintEvent>
#include <QPainter>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QTableView>
#include <QLayout>

#include "qdatacube.h"
#include "qdatacube_header_model.h"
#include "qdatacube_model.h"

namespace qdatacube {

qdatacube_header_t::qdatacube_header_t(Qt::Orientation orientation, QWidget* parent):
    QHeaderView(orientation, parent),
    m_layout(0L){
  if (orientation == Qt::Horizontal) {
    m_layout = new QVBoxLayout(this);
  } else {
    m_layout = new QHBoxLayout(this);
  }
  m_layout->setSpacing(0);
  m_layout->setMargin(0);
  connect(this, SIGNAL(sectionResized(int,int,int)), this, SLOT(slotSectionResized(int,int,int)));
  QAbstractScrollArea* scroll_area =  dynamic_cast<QAbstractScrollArea*>(parent);
  connect(orientation == Qt::Horizontal ? scroll_area->horizontalScrollBar() : scroll_area->verticalScrollBar(),
          SIGNAL(valueChanged(int)), this, SLOT(setAllOffset(int)));
}

QSize qdatacube_header_t::sizeHint() const {
  QSize rv;
  if (orientation() == Qt::Horizontal) {
    foreach (QHeaderView* headerview, m_headers) {
      QSize size = headerview->sizeHint();
      rv.setHeight(std::max(rv.height()+size.height(),20));
    }
    if (!m_headers.isEmpty()) {
      rv.setWidth(std::max(m_headers.last()->width(),100));
    }
  } else {
    foreach (QHeaderView* headerview, m_headers) {
      QSize size = headerview->sizeHint();
      rv.setWidth(std::max(rv.width()+size.width(),10));
    }
    if (!m_headers.isEmpty()) {
      rv.setHeight(std::max(m_headers.last()->height(),200));
    }
  }
  return rv;
}

void qdatacube_header_t::setModel(QAbstractItemModel* model) {
  QHeaderView::setModel(model);
}

void qdatacube_header_t::setAllOffset(int value) {
  QAbstractScrollArea* scroll_area =  dynamic_cast<QAbstractScrollArea*>(parent());
  if (!scroll_area) {
    return;
  }
  QScrollBar* bar = orientation() == Qt::Horizontal ? scroll_area->horizontalScrollBar() : scroll_area->verticalScrollBar();
  Q_ASSERT(bar);
  int max = bar->maximum();
  if (value != max) {
    foreach (QHeaderView* headerview, m_headers) {
      headerview->setOffsetToSectionPosition(value);
    }
  } else if (max>0) {
    foreach (QHeaderView* headerview, m_headers) {
      headerview->setOffsetToLastSection();
    }
  }
}

void qdatacube_header_t::updateSizes() {
  if (m_headers.empty()) {
    return;
  }
  QHeaderView* lastheader = m_headers.last();
  for (int index = 0; index<m_headers.size(); ++index) {
    QHeaderView* headerview = m_headers[index];
    if (headerview != lastheader) {
      int lhindex = 0;
      for (int j=0;j<m_headers[index]->count();++j) {
        int span = headerview->model()->headerData(j, orientation(), Qt::UserRole+1).toInt();
        int size = 0;
        for (int k=0; k<span; ++k) {
          size += lastheader->sectionSize(lhindex++);
        }
        headerview->resizeSection(j, size);
        m_headers[index]->resizeSection(j,size);
      }
    }
    ++index;
  }

}

void qdatacube_header_t::setSectionSize(int section,int oldsize, int size) {
  if (m_lock) {
    return;
  }
  m_lock = true;
  if (QHeaderView* header = qobject_cast<QHeaderView*>(sender())) {
    int index = m_headers.indexOf(header);
    if (index < m_headers.size()-1) {
      int lhindex = 0;
      for (int i=0;i<section;++i) {
        int span = header->model()->headerData(i, orientation(), Qt::UserRole+1).toInt();
        lhindex+=span;
      }
      int span = header->model()->headerData(section, orientation(), Qt::UserRole+1).toInt();
      for (int i=0; i<span;++i) {
        int oldlhsize = m_headers.last()->sectionSize(i+lhindex);
        int newlhsize = oldlhsize + getnewlhsize(oldsize, size, i, span );

        m_headers.last()->resizeSection(lhindex+i, newlhsize);
        resizeSection(lhindex+i, newlhsize);
      }
    } else {
      resizeSection(section, size);
    }
  }
  updateSizes();
  m_lock = false;
}

int qdatacube_header_t::getnewlhsize(int oldsize, int size, int i, int span) {
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

void qdatacube_header_t::slotSectionResized(int section, int /*oldsize*/, int size) {
  m_headers.last()->resizeSection(section, size);
}

void qdatacube_header_t::reset() {
  const Qt::Orientation orientation = this->orientation();
  foreach(QHeaderView* header, m_headers) {
    QAbstractItemModel* header_model = header->model();
    delete header;
    delete header_model;
  }
  m_headers.clear();
  if (qdatacube_model_t* qdatacube_model = qobject_cast<qdatacube_model_t*>(model())) {
    qdatacube_t* qdatacube = qdatacube_model->qdatacube();
    for (int i=0, iend = qdatacube->depth(orientation); i<iend; ++i) {

      QHeaderView* headerview = new QHeaderView(orientation, this);
      m_layout->addWidget(headerview);
      connect(headerview, SIGNAL(sectionResized(int,int,int)), SLOT(setSectionSize(int,int,int)));
      m_headers << headerview;
      if (qdatacube_model) {
        qdatacube_header_model_t* rhm = new qdatacube_header_model_t(qdatacube_model, orientation, i);
        connect(rhm,SIGNAL(modelReset()),this,SLOT(reset()));
        headerview->setModel(rhm);
      }

    }
  } else {
    return;
  }
  // Set minimum sizes for headers
  QHeaderView* lastheader = m_headers.last();
  Q_ASSERT(lastheader);
  for (int section=0; section < lastheader->count(); ++section) {
    setMinimumSectionSize(std::max(lastheader->minimumSectionSize(),20));
  }
  foreach (QHeaderView* header, m_headers) {
    if (header != lastheader) {
      for (int i=0; i<header->count(); ++i) {
        int span = header->model()->headerData(i, orientation, Qt::UserRole+1).toInt();
        int minsize = qMax(qMax(span*lastheader->minimumSectionSize(), header->minimumSectionSize()),30);
        int key = m_headers.indexOf(header);
        header->setMinimumSectionSize(minsize);
        m_headers.value(key)->setMinimumSectionSize(minsize);

      }
    }
  }
  m_lock = true;
  for (int i=0; i<count();++i) {
    m_headers.last()->resizeSection(i,sectionSize(i));
    lastheader->resizeSection(i, sectionSize(i));
    m_headers.last()->showSection(i);
  }

  updateSizes();
  m_lock = false;
  m_layout->update();
  if(size().width() ==0){
    resize(20,size().height());
  }
  if(size().height() == 0){
    resize(size().width(),40);
  }
  QHeaderView::reset();

}

void qdatacube_header_t::paintEvent(QPaintEvent* e) {
  QWidget::paintEvent(e);
}

int qdatacube_header_t::sizeHintForColumn(int ) const {
  return -1; // Perhaps TODO, but this avoid unforunate recursion
}

int qdatacube_header_t::sizeHintForRow(int ) const {
  return -1; // Perhaps TODO, but this avoid unforunate recursion
}
}
#include "qdatacube_header.moc"
