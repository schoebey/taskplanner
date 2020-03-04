#include "tagwidget.h"
#include "styleExtension.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>

#include <cmath>

template<> TagWidget* Draggable<TagWidget>::m_pDraggingInstance = nullptr;

TagWidget::TagWidget(const QString& sText, QWidget* pParent)
  : QLabel(sText, pParent),
    Draggable<TagWidget>(this)
{
}

void TagWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  TODO: own style option with text and other stuff
  QStyleOptionFrame opt;
  opt.initFrom(this);

  QPainter painter(this);
  style()->drawControl(customControlElements::CE_TagWidget, &opt, &painter, this);
}

