#include "layoututils.h"
#include "flowlayout.h"

#include <QWidget>
#include <QGridLayout>
#include <QPoint>
#include <QDebug>

#include <cmath>

namespace tools
{
double dist(const QPoint& p1, const QPoint& p2)
{
   double dX = p1.x() - p2.x();
   double dY = p1.y() - p2.y();
   return sqrt(dX * dX + dY * dY);
}

bool addToGridLayout(QWidget* pT,
                     QGridLayout* pLayout,
                     QPoint pt)
{
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
          QPoint ptCenter = pWidget->mapTo(pWidget->parentWidget(),
                                           pWidget->rect().center());

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

bool addToFlowLayout(QWidget* pT, FlowLayout* pLayout, QPoint pt)
{
  if (nullptr != pLayout)
  {
    int iPos = 0;
    std::vector<QWidget*> vpWidgets;
    double dMinDist = std::numeric_limits<double>::max();
    for (int i = 0; i < pLayout->count(); ++i)
    {
      auto pItem = pLayout->itemAt(i);
      if (nullptr != pItem) {
        QWidget* pWidget = pItem->widget();

        vpWidgets.push_back(pWidget);
        QPoint ptCenter = pWidget->mapTo(pWidget->parentWidget(),
                                         pWidget->rect().center());

        double dDist = dist(pt, ptCenter);
        if (dDist < dMinDist)
        {
          dMinDist = dDist;
          iPos = pt.x() < ptCenter.x() ? i : i + 1;
        }
      }
    }

    while (!pLayout->isEmpty())
    {
      QLayoutItem* pItem = pLayout->takeAt(0);
      pItem->widget()->setParent(nullptr);
      delete pItem;
    }
    vpWidgets.insert(vpWidgets.begin() + iPos, pT);

    for (const auto& el : vpWidgets)
    {
      pLayout->addWidget(el);

      el->setVisible(true);
    }


    return true;
  }
  return false;
}
}
