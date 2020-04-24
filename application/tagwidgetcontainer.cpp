#include "tagwidgetcontainer.h"
#include "flowlayout.h"
#include "layoututils.h"

#include <QLayout>

template<> std::vector<DraggableContainer<DraggableTagWidget>*>
DraggableContainer<DraggableTagWidget>::m_vpMouseOverContainers = std::vector<DraggableContainer<DraggableTagWidget>*>();

TagWidgetContainer::TagWidgetContainer(QWidget* pParent)
  : DraggableContainer<DraggableTagWidget>(pParent),
    m_pPlaceholder(new QFrame())
{
  m_pPlaceholder->setObjectName("tag_placeholder");
  QLayout* pLayout = new FlowLayout(this, 0, 0, 0);
  setLayout(pLayout);
}

bool TagWidgetContainer::addItem_impl(DraggableTagWidget* pT)
{
  QLayout* pLayout = layout();
  if (nullptr != pLayout)
  {
    pLayout->addWidget(pT);
    emit tagAdded(pT);
    return true;
  }

  return false;
}

bool TagWidgetContainer::removeItem_impl(DraggableTagWidget* pT)
{
  emit tagRemoved(pT);
  return true;
}

bool TagWidgetContainer::insertItem_impl(DraggableTagWidget* pT, QPoint pt)
{
  m_pPlaceholder->setParent(nullptr);
  m_pPlaceholder->hide();
  FlowLayout* pLayout = dynamic_cast<FlowLayout*>(layout());
  if (tools::addToFlowLayout(pT, pLayout, pt))
  {
    emit tagAdded(pT);
    return true;
  }

  return false;
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
