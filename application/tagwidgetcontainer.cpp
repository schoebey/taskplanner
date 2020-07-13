#include "tagwidgetcontainer.h"
#include "flowlayout.h"
#include "layoututils.h"
#include "tagwidget.h"

#include <QLayout>
#include <QPushButton>
#include <QWidgetAction>

template<> std::vector<QPointer<DraggableContainer<DraggableTagWidget>>>
DraggableContainer<DraggableTagWidget>::m_vpMouseOverContainers = std::vector<QPointer<DraggableContainer<DraggableTagWidget>>>();

namespace {
  void wireTag(TagWidgetContainer* pContainer, TagWidget* pTagWidget)
  {
    QObject::connect(pTagWidget, &TagWidget::textChanged, pContainer,
                     [pTagWidget, pContainer](const QString& sNew)
    {
      emit pContainer->tagChanged(pTagWidget->id(), sNew, pTagWidget->color());
    });
    QObject::connect(pTagWidget, &TagWidget::colorChanged, pContainer,
                     [pTagWidget, pContainer]()
    {
      emit pContainer->tagChanged(pTagWidget->id(), pTagWidget->text(), pTagWidget->color());
    });
  }
}

TagWidgetContainer::TagWidgetContainer(QWidget* pParent)
  : DraggableContainer<DraggableTagWidget>(pParent),
    m_pPlaceholder(new QFrame())
{
  m_pPlaceholder->setObjectName("tag_placeholder");
  QLayout* pLayout = new FlowLayout(this, 0, 1, 1);
  setLayout(pLayout);

  // contextmenu to add new tags
  QAction* pAction = new QAction(tr("add tag"), this);
  addAction(pAction);
  connect(pAction, &QAction::triggered, this, [this]()
  {
    emit newTagRequested("new tag", this);
  });

  setContextMenuPolicy(Qt::ActionsContextMenu);
}

void TagWidgetContainer::setEditable(bool bEditable)
{
  m_bEditable = bEditable;
}

bool TagWidgetContainer::modifyTag(tag_id id, const QString& sNewName, const QColor& col)
{
  for (auto& pWidget : items())
  {
    if (pWidget->id() == id)
    {
      pWidget->setText(sNewName);
      pWidget->setColor(col);
      return true;
    }
  }
  return false;
}

void TagWidgetContainer::clear()
{
  std::vector<QPointer<DraggableTagWidget>> vItems = items();
  for (auto& el : vItems)
  {
    removeItem(el);
    delete el;
  }
}

bool TagWidgetContainer::addItem_impl(DraggableTagWidget* pT)
{
  QLayout* pLayout = layout();
  if (nullptr != pLayout)
  {
    pLayout->addWidget(pT);
    pT->setEditable(m_bEditable);
    if (m_bEditable) { wireTag(this, pT); }
    return true;
  }

  return false;
}

bool TagWidgetContainer::removeItem_impl(DraggableTagWidget* pT)
{
  QLayout* pLayout = layout();
  if (nullptr != pLayout)
  {
    pLayout->removeWidget(pT);
    return true;
  }

  return false;
}

bool TagWidgetContainer::insertItem_impl(DraggableTagWidget* pT, QPoint pt)
{
  m_pPlaceholder->setParent(nullptr);
  m_pPlaceholder->hide();
  FlowLayout* pLayout = dynamic_cast<FlowLayout*>(layout());
  if (tools::addToFlowLayout(pT, pLayout, pt))
  {
    pT->setEditable(m_bEditable);
    if (m_bEditable) { wireTag(this, pT); }
    return true;
  }

  return false;
}

bool TagWidgetContainer::moveItemFrom_impl(DraggableContainer<DraggableTagWidget>* /*pSource*/,
                                           DraggableTagWidget* pT, QPoint pt)
{
  m_pPlaceholder->setParent(nullptr);
  m_pPlaceholder->hide();
  FlowLayout* pLayout = dynamic_cast<FlowLayout*>(layout());
  if (tools::addToFlowLayout(pT, pLayout, pt))
  {
    pT->setEditable(m_bEditable);
    if (m_bEditable) { wireTag(this, pT); }
    return true;
  }

  return false;
}

bool TagWidgetContainer::showPlaceholderAt_impl(const QPoint& pt, const QSize& s)
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

void TagWidgetContainer::emitItemAdded(DraggableTagWidget* pT)
{
  emit tagAdded(pT);
}

void TagWidgetContainer::emitItemInserted(DraggableTagWidget* pT, QPoint)
{
  emit tagAdded(pT);
}

void TagWidgetContainer::emitItemRemoved(DraggableTagWidget* pT)
{
  emit tagRemoved(pT);
}

void TagWidgetContainer::emitItemMovedFrom(DraggableTagWidget* pT,
                                       DraggableContainer<DraggableTagWidget>* pSource)
{
  emit tagMoved(pT, pSource);
}
