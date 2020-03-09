#include "tagwidgetcontainer.h"
#include "flowlayout.h"
#include "layoututils.h"

#include <QLayout>

template<> std::set<DraggableContainer<DraggableTagWidget>*>
DraggableContainer<DraggableTagWidget>::m_visibleContainers = std::set<DraggableContainer<DraggableTagWidget>*>();

TagWidgetContainer::TagWidgetContainer(QWidget* pParent)
  : DraggableContainer<DraggableTagWidget>(pParent),
    m_pPlaceholder(new QFrame())
{
  QLayout* pLayout = new FlowLayout(this);
  setLayout(pLayout);
  m_pPlaceholder->setStyleSheet("border: 1px dashed red; background: green;");
}

bool TagWidgetContainer::addItem_impl(DraggableTagWidget* pT)
{
  QLayout* pLayout = layout();
  if (nullptr != pLayout)
  {
    pLayout->addWidget(pT);
    return true;
  }

  return false;
}

bool TagWidgetContainer::removeItem_impl(DraggableTagWidget* pT)
{
  return true;
}

bool TagWidgetContainer::insertItem_impl(DraggableTagWidget* pT, QPoint pt)
{
  m_pPlaceholder->setParent(nullptr);
  m_pPlaceholder->hide();
  FlowLayout* pLayout = dynamic_cast<FlowLayout*>(layout());
  return tools::addToFlowLayout(pT, pLayout, pt);
}

bool TagWidgetContainer::showPlaceholderAt(const QPoint& pt, const QSize& s)
{
  m_pPlaceholder->setParent(nullptr);
  m_pPlaceholder->setMinimumSize(s);

  FlowLayout* pLayout = dynamic_cast<FlowLayout*>(layout());
  if (tools::addToFlowLayout(m_pPlaceholder, pLayout, pt))
  {
    m_pPlaceholder->show();
    return true;
  }

  m_pPlaceholder->hide();
  return false;
}

void TagWidgetContainer::hidePlaceholder()
{
  m_pPlaceholder->setParent(nullptr);
  m_pPlaceholder->hide();
}
