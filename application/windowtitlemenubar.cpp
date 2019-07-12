#include "windowtitlemenubar.h"
#include "widgetresizer.h"

#include <QMouseEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>

WindowTitleMenuBar::WindowTitleMenuBar(QWidget *pParent)
  : QMenuBar(pParent)
{
#ifndef Q_OS_MAC
  m_pTitle = new QLabel("hello world", this);
  m_pRightButtonBox = new QFrame(this);
  QHBoxLayout* pLayout = new QHBoxLayout(m_pRightButtonBox);
  m_pRightButtonBox->setLayout(pLayout);

  auto pButton = new QPushButton("_", this);
  connect(pButton, &QPushButton::clicked, this, &WindowTitleMenuBar::minimize);
  addWidget(pButton);

  m_pMaximizeButton = new QPushButton("M", this);
  m_pMaximizeButton->setCheckable(true);
  connect(m_pMaximizeButton, &QPushButton::toggled, this, &WindowTitleMenuBar::toggleMaximize);
  addWidget(m_pMaximizeButton);

  pButton = new QPushButton("X", this);
  connect(pButton, &QPushButton::clicked, this, &WindowTitleMenuBar::close);
  addWidget(pButton);

  QMetaObject::invokeMethod(this, "updateWidgets", Qt::QueuedConnection);

  window()->installEventFilter(this);

  new WidgetResizer(window(), WidgetResizer::eTopLeft);
  new WidgetResizer(window(), WidgetResizer::eTop);
  new WidgetResizer(window(), WidgetResizer::eTopRight);
  new WidgetResizer(window(), WidgetResizer::eLeft);
  new WidgetResizer(window(), WidgetResizer::eRight);
  new WidgetResizer(window(), WidgetResizer::eBottomLeft);
  new WidgetResizer(window(), WidgetResizer::eBottom);
  new WidgetResizer(window(), WidgetResizer::eBottomRight);
#endif
}

#ifndef Q_OS_MAC
void WindowTitleMenuBar::addWidget(QWidget *pWidget)
{
  m_pRightButtonBox->layout()->addWidget(pWidget);
}

bool WindowTitleMenuBar::eventFilter(QObject* pObject, QEvent* pEvent)
{
  if (pObject == window())
  {
    switch (pEvent->type())
    {
    case QEvent::WindowTitleChange:
      updateWidgets();
      break;
    case QEvent::ModifiedChange:
      updateWidgets();
      break;
    case QEvent::WindowStateChange:
      updateWidgets();
    default:
      break;
    }
  }

  return false;
}

void WindowTitleMenuBar::toggleMaximize(bool bMaximize)
{
  if (bMaximize)  { maximize(); }
  else            { restore(); }
}

void WindowTitleMenuBar::maximize()
{
  window()->setWindowState(Qt::WindowMaximized);
}

void WindowTitleMenuBar::minimize()
{
  window()->setWindowState(Qt::WindowMinimized);
}

void WindowTitleMenuBar::restore()
{
  window()->setWindowState(Qt::WindowNoState);
}

void WindowTitleMenuBar::close()
{
  window()->close();
}

void WindowTitleMenuBar::mousePressEvent(QMouseEvent* pEvent)
{
  QMenuBar::mousePressEvent(pEvent);


  if (nullptr == actionAt(pEvent->pos()))
  {
    m_bDraggingWindow = true;
    m_mouseDownPoint = pEvent->pos();
  }
}

QPoint WindowTitleMenuBar::stickToScreenBorders(const QPoint& targetPos, int iMagneticDistance)
{
  auto pDesktop = QApplication::desktop();
  auto screenGeo = pDesktop->availableGeometry(pDesktop->screenNumber(this));

  // check if window is near desktop border and magnetically stick to it
  bool bTearOffX = abs((targetPos - m_mouseDownPoint).x()) > 2 * iMagneticDistance;
  bool bTearOffY = abs((targetPos - m_mouseDownPoint).y()) > 2 * iMagneticDistance;
  int iLeft = window()->x();
  int iRight = iLeft + window()->width();
  int iTop = window()->y();
  int iBottom = iTop + window()->height();

  QPoint position = mapToGlobal(targetPos) - m_mouseDownPoint;
  if (!bTearOffX )
  {
    if (iLeft < screenGeo.left() + iMagneticDistance && iLeft >= screenGeo.left())
    {
      position.setX(screenGeo.left());
    }
    else if (iRight > screenGeo.right() - iMagneticDistance && iRight <= screenGeo.right() + 1)
    {
      position.setX(screenGeo.right() - window()->width() + 1);
    }
  }

  if (!bTearOffY)
  {
    if (iTop < screenGeo.top() + iMagneticDistance && iTop >= screenGeo.top())
    {
      position.setY(screenGeo.top());
    }
    else if (iBottom > screenGeo.bottom() - iMagneticDistance && iBottom <= screenGeo.bottom() + 1)
    {
      position.setY(screenGeo.bottom() - window()->height() + 1);
    }
  }

  return position;
}

void WindowTitleMenuBar::mouseMoveEvent(QMouseEvent* pEvent)
{
  QMenuBar::mouseMoveEvent(pEvent);

  if (m_bDraggingWindow)
  {
    if (window()->windowState() & Qt::WindowMaximized)
    {
      restore();

      // TODO: recalc mousedown point so it feels more 'natural'
    }


    QPoint position = stickToScreenBorders(pEvent->pos(), 50);

    window()->move(position);
  }
}

void WindowTitleMenuBar::mouseDoubleClickEvent(QMouseEvent* /*pEvent*/)
{
  toggleMaximize(!(window()->windowState() & Qt::WindowMaximized));
}

void WindowTitleMenuBar::mouseReleaseEvent(QMouseEvent* pEvent)
{
  QMenuBar::mouseReleaseEvent(pEvent);

  m_bDraggingWindow = false;
}

void WindowTitleMenuBar::updateWidgets()
{
  QString sTitle = window()->windowTitle();
  sTitle.replace("[*]", window()->isWindowModified() ? "*" :  "");
  m_pTitle->setText(sTitle);

  m_pTitle->resize(m_pTitle->sizeHint());
  m_pTitle->move(rect().center().x() - m_pTitle->width() / 2, 0);

  m_pRightButtonBox->move(width() - m_pRightButtonBox->width(), 0);

  m_pMaximizeButton->setChecked(window()->windowState() & Qt::WindowMaximized);
}

void WindowTitleMenuBar::resizeEvent(QResizeEvent* /*pEvent*/)
{
  updateWidgets();
}
#endif
