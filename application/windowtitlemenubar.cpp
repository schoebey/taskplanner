#include "windowtitlemenubar.h"

#include <QMouseEvent>
#include <QHBoxLayout>
#include <QPushButton>

WindowTitleMenuBar::WindowTitleMenuBar(QWidget *pParent)
  : QMenuBar(pParent)
{
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
}

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

    window()->move(pEvent->globalPos() - m_mouseDownPoint);
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
  m_pTitle->move(rect().center().x() - m_pTitle->width() / 2, 0);

  m_pRightButtonBox->move(width() - m_pRightButtonBox->width(), 0);

  m_pMaximizeButton->setChecked(window()->windowState() & Qt::WindowMaximized);
}

void WindowTitleMenuBar::resizeEvent(QResizeEvent* /*pEvent*/)
{
  updateWidgets();
}
