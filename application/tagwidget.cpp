#include "tagwidget.h"
#include "styleExtension.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>

#include <cmath>

QStyleOptionTagWidget::QStyleOptionTagWidget()
  : QStyleOption(1, customStyleOptions::SO_TagWidget)
{

}

template<> TagWidget* Draggable<TagWidget>::m_pDraggingInstance = nullptr;

TagWidget::TagWidget(const QString& sText, QWidget* pParent)
  : QLabel(sText, pParent),
    Draggable<TagWidget>(this)
{
}

QSize TagWidget::sizeHint() const
{
  int iHeaderSize = style()->pixelMetric(customPixelMetrics::PM_TagHeader);
  int iBorderSize = style()->pixelMetric(customPixelMetrics::PM_TagBorder);

  return QLabel::sizeHint() + QSize(iHeaderSize, 0) + QSize(2 * iBorderSize, 2 * iBorderSize);
}

void TagWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  QStyleOptionTagWidget opt;
  opt.initFrom(this);

  opt.sText = text();
  opt.color = Qt::yellow;

  QPainter painter(this);
  style()->drawControl(customControlElements::CE_TagWidget, &opt, &painter, this);
}


