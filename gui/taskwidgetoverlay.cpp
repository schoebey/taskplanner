#include "taskwidgetoverlay.h"
#include "taskwidget.h"

#include <QPaintEvent>
#include <QPainter>

TaskWidgetOverlay::TaskWidgetOverlay(TaskWidget *parent)
  : QWidget(parent),
    m_pParent(parent)
{
  setAttribute(Qt::WA_TransparentForMouseEvents);
}

void TaskWidgetOverlay::paintEvent(QPaintEvent* pEvent)
{
  QPainter painter(this);

  painter.setPen(Qt::red);
  painter.setBrush(QColor(255,0,0,50));
  painter.drawRoundedRect(rect(), 5, 5);
}


