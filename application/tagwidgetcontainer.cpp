#include "tagwidgetcontainer.h"

#include <QLayout>

namespace
{
  double dist(const QPoint& p1, const QPoint& p2)
  {
     double dX = p1.x() - p2.x();
     double dY = p1.y() - p2.y();
     return sqrt(dX * dX + dY * dY);
  }
}

template<> DraggableContainer<TagWidget>* DraggableContainer<TagWidget>::m_pContainerUnderMouse = nullptr;
TagWidgetContainer::TagWidgetContainer(QWidget* pParent)
  : QFrame(pParent),
    DraggableContainer<TagWidget>(this)
{

}

bool TagWidgetContainer::addItem_impl(TagWidget* pT)
{
  QLayout* pLayout = layout();
  if (nullptr != pLayout)
  {
    pLayout->addWidget(pT);
    return true;
  }

  return false;
}

bool TagWidgetContainer::removeItem_impl(TagWidget* pT)
{
  return true;
}

bool TagWidgetContainer::insertItem_impl(TagWidget* pT, QPoint pt)
{
  QGridLayout* pLayout = dynamic_cast<QGridLayout*>(layout());
  if (nullptr != pLayout)
  {
    int iRow = -1;
    int iColumn = -1;
    double dMinDist = std::numeric_limits<double>::max();
    for (int i = 0; i < pLayout->rowCount(); ++i)
    {
      for (int j = 0; j < pLayout->columnCount(); ++j)
      {
        auto pItem = pLayout->itemAtPosition(i, j);
        if (nullptr != pItem) {
          QWidget* pWidget = pItem->widget();
          QPoint ptCenter = pWidget->mapTo(this, pWidget->rect().center());

          double dDist = dist(pt, ptCenter);
          if (dDist < dMinDist)
          {
            dMinDist = dDist;
            iRow = pt.x() < ptCenter.x() ? i : i + 1;
            iColumn = pt.y() < ptCenter.y() ? j : j + 1;
          }
        }
      }
    }

    pLayout->addWidget(pT, iRow, iColumn);
    return true;
  }
  return false;
}
