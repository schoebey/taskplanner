#include "widgetresizer.h"
#include "styleExtension.h"

#include <QMouseEvent>
#include <QStyle>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

WidgetResizer::WidgetResizer(QWidget* pParent, EPosition anchor)
  : QFrame(pParent),
    m_pParent(pParent),
    m_bMouseDown(false),
    m_anchor(anchor)
{
  const int iActiveSize = style()->pixelMetric(customPixelMetrics::PM_ResizerActiveArea);
  resize(iActiveSize, iActiveSize);
  raise();
  setMouseTracking(true);


  update(true);
}

void WidgetResizer::mousePressEvent(QMouseEvent* pEvent)
{
  if (!isEnabled())  { return; }

  m_bMouseDown = true;

  QPoint pt(mapToParent(pEvent->pos()));
  switch (m_anchor)
  {
  case eTopLeft:
    m_mouseDownPt = pt;
    break;
  case eTop:
    m_mouseDownPt = pt;
    break;
  case eTopRight:
    m_mouseDownPt = QPoint(m_pParent->width() - pt.x(), pt.y());
    break;
  case eRight:
    m_mouseDownPt = QPoint(m_pParent->width() - pt.x(), pt.y());
    break;
  case eBottomRight:
    m_mouseDownPt = QPoint(m_pParent->width(), m_pParent->height()) - pt;
    break;
  case eBottom:
    m_mouseDownPt = QPoint(pt.x(), m_pParent->height() - pt.y());
    break;
  case eBottomLeft:
    m_mouseDownPt = QPoint(pt.x(), m_pParent->height() - pt.y());
    break;
  case eLeft:
    m_mouseDownPt = pt;
    break;
  case eCenter:
  default:
    pEvent->ignore();
    break;
  }
}

void WidgetResizer::mouseDoubleClickEvent(QMouseEvent *pEvent)
{
  if (!isEnabled())  { return; }

  auto pDesktop = QApplication::desktop();
  int i = pDesktop->screenNumber(this);
  auto screens = QGuiApplication::screens();
  if (screens.size() > i)
  {
    QScreen* pScreen = screens.at(pDesktop->screenNumber(this));
    auto screenGeo = pScreen->availableGeometry();;

    switch (m_anchor)
    {
    case eTopLeft:
    case eTopRight:
    case eBottomRight:
    case eBottomLeft:
        break;
    case eTop:
    case eBottom:
        m_pParent->move(m_pParent->pos().x(), screenGeo.y());
        m_pParent->resize(m_pParent->size().width(), screenGeo.height());
        break;
    case eRight:
    case eLeft:
        m_pParent->move(screenGeo.x(), m_pParent->pos().y());
        m_pParent->resize(screenGeo.width(), m_pParent->size().height());
        break;
    case eCenter:
    default:
        pEvent->ignore();
        break;
    }
  }
}

void WidgetResizer::mouseReleaseEvent(QMouseEvent* pEvent)
{
  if (!isEnabled())  { return; }

  m_bMouseDown = false;
  pEvent->ignore();
}

void WidgetResizer::mouseMoveEvent(QMouseEvent* pEvent)
{
  if (!isEnabled())  { return; }

  switch (m_anchor)
  {
  case eTopLeft:
    setCursor(Qt::SizeFDiagCursor);
    break;
  case eTop:
    setCursor(Qt::SizeVerCursor);
    break;
  case eTopRight:
    setCursor(Qt::SizeBDiagCursor);
    break;
  case eRight:
    setCursor(Qt::SizeHorCursor);
    break;
  case eBottomRight:
    setCursor(Qt::SizeFDiagCursor);
    break;
  case eBottom:
    setCursor(Qt::SizeVerCursor);
    break;
  case eBottomLeft:
    setCursor(Qt::SizeBDiagCursor);
    break;
  case eLeft:
    setCursor(Qt::SizeHorCursor);
    break;
  case eCenter:
  default:
    setCursor(Qt::ArrowCursor);
    pEvent->ignore();
    break;
  }

  if (m_bMouseDown)
  {
    switch (m_anchor)
    {
    case eTopLeft:
      {
        QPoint pt(mapToParent(pEvent->pos()) - m_mouseDownPt);
        QSize cur(m_pParent->size());
        m_pParent->resize(m_pParent->size() - QSize(pt.x(), pt.y()));
        QSize delta(cur - m_pParent->size());
        m_pParent->move(m_pParent->pos() + QPoint(delta.width(), delta.height()));
      }
      break;
    case eTop:
      {
        QPoint pt(mapToParent(pEvent->pos()) - m_mouseDownPt);
        QSize cur(m_pParent->size());
        m_pParent->resize(m_pParent->size() - QSize(0, pt.y()));
        QSize delta(cur - m_pParent->size());
        m_pParent->move(m_pParent->pos() + QPoint(0, delta.height()));
      }
      break;
    case eTopRight:
      {
        QPoint pt(mapToParent(pEvent->pos()));
        QSize cur(m_pParent->size());
        m_pParent->resize(pt.x() + m_mouseDownPt.x(), m_pParent->height() - pt.y() + m_mouseDownPt.y());
        QSize delta(cur - m_pParent->size());
        m_pParent->move(m_pParent->pos() + QPoint(0, delta.height()));
      }
      break;
    case eRight:
      {
        QPoint newSize = mapToParent(pEvent->pos()) + m_mouseDownPt;
        m_pParent->resize(QSize(newSize.x(), m_pParent->height()));
      }
      break;
    case eBottomRight:
      {
        QPoint newSize = mapToParent(pEvent->pos()) + m_mouseDownPt;
        m_pParent->resize(QSize(newSize.x(), newSize.y()));
      }
      break;
    case eBottom:
      {
        QPoint newSize = mapToParent(pEvent->pos()) + m_mouseDownPt;
        m_pParent->resize(QSize(m_pParent->width(), newSize.y()));
      }
      break;
    case eBottomLeft:
      {
        QPoint pt(mapToParent(pEvent->pos()));
        QSize cur(m_pParent->size());
        m_pParent->resize(m_pParent->width() - pt.x() + m_mouseDownPt.x(), pt.y() + m_mouseDownPt.y());
        QSize delta(cur - m_pParent->size());
        m_pParent->move(m_pParent->pos() + QPoint(delta.width(), 0));
      }
      break;
    case eLeft:
      {
        QPoint pt(mapToParent(pEvent->pos()));
        QSize cur(m_pParent->size());
        m_pParent->resize(m_pParent->width() - pt.x() + m_mouseDownPt.x(), m_pParent->height());
        QSize delta(cur - m_pParent->size());
        m_pParent->move(m_pParent->pos() + QPoint(delta.width(), 0));
      }
      break;
    case eCenter:
      break;
    default:
      break;
    }
  }
}

bool WidgetResizer::eventFilter(QObject* pObj, QEvent* pEvent)
{
  if (!isVisible())  { return false; }

  if (pObj == m_pParent)
  {
    switch (pEvent->type())
    {
    case QEvent::Resize:
    case QEvent::Show:
      update(true);
      break;
    default:
      break;
    }
  }

  return false;
}

void WidgetResizer::update(bool bVisible)
{
  if (bVisible ^ isVisible())
  {
    if (bVisible)
    {
      m_pParent->installEventFilter(this);
    }
    else
    {
      m_pParent->removeEventFilter(this);
    }
    setVisible(bVisible);
    setEnabled(bVisible);
  }

  const int iBorderSize = style()->pixelMetric(customPixelMetrics::PM_FloatingBorderSize);
  const int iActiveSize = style()->pixelMetric(customPixelMetrics::PM_ResizerActiveArea);
  switch (m_anchor)
  {
  case eTopLeft:
    move(iBorderSize, iBorderSize);
    break;
  case eTop:
    move(iActiveSize + iBorderSize, iBorderSize);
    resize(m_pParent->width() - 2 * iActiveSize - 2 * iBorderSize, iActiveSize);
    break;
  case eTopRight:
    move(m_pParent->width() - iActiveSize - iBorderSize, iBorderSize);
    break;
  case eRight:
    move(m_pParent->width() - iActiveSize - iBorderSize, iActiveSize + iBorderSize);
    resize(iActiveSize, m_pParent->height() - 2 * iActiveSize - 2 * iBorderSize);
    break;
  case eBottomRight:
    move(m_pParent->width() - iActiveSize - iBorderSize, m_pParent->height() - iActiveSize - iBorderSize);
    break;
  case eBottom:
    move(iActiveSize + iBorderSize, m_pParent->height() - iActiveSize - iBorderSize);
    resize(m_pParent->width() - 2 * iActiveSize - 2 * iBorderSize, iActiveSize);
    break;
  case eBottomLeft:
    move(iBorderSize, m_pParent->height() - iActiveSize - iBorderSize);
    break;
  case eLeft:
    move(iBorderSize, iActiveSize + iBorderSize);
    resize(iActiveSize, m_pParent->height() - 2 * iActiveSize - 2 * iBorderSize);
    break;
  case eCenter:
    break;
  default:
    break;
  }


  raise();
}

