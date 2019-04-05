#include "mousehandlingframe.h"

#include <QMouseEvent>

MouseHandlingFrame::MouseHandlingFrame(QWidget* pParent)
 : QFrame(pParent)
{

}

MouseHandlingFrame::~MouseHandlingFrame()
{

}

void MouseHandlingFrame::mousePressEvent(QMouseEvent* pEvent)
{
  m_mouseDownPt = pEvent->pos();
}

void MouseHandlingFrame::mouseReleaseEvent(QMouseEvent* /*pEvent*/)
{
  emit mouseClicked(m_mouseDownPt);
  m_mouseDownPt = QPoint();
}

void MouseHandlingFrame::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
  emit mouseDoubleClicked(pEvent->pos());
}

