#include "tagwidget.h"
#include "styleExtension.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>

#include <cmath>


TagWidget::TagWidget(const QString& sText, QWidget* pParent)
  : QLabel(sText, pParent),
    Draggable<TagWidget> (this)
{

}

void TagWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  QStyleOptionFrame opt;
  opt.initFrom(this);

  QPainter painter(this);
  style()->drawControl(customControlElements::CE_TagWidget, &opt, &painter, this);
}

void TagWidget::mouseMoveEvent(QMouseEvent* pMouseEvent)
{
  QFrame::mouseMoveEvent(pMouseEvent);

  onMouseMove(pMouseEvent);
}

void TagWidget::mousePressEvent(QMouseEvent* pMouseEvent)
{
  setMouseDown(true, pMouseEvent->pos());
}

void TagWidget::mouseReleaseEvent(QMouseEvent* /*pMouseEvent*/)
{
  setMouseDown(false);
}
