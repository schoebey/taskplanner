#include "overlaywidget.h"
#include <QLayout>
#include <QPushButton>
#include <QEvent>
#include <QLabel>
#include <QStyle>
#include <QPropertyAnimation>
#include <QPaintEvent>
#include <QAction>

OverlayWidget::OverlayWidget(QWidget *parent)
  : QFrame(nullptr != parent ? parent->window() : nullptr),
    m_pParent(parent),
    m_pLayout(new QGridLayout(this)),
    m_pCloseButton(new QPushButton("X", this))
{
  setFocusPolicy(Qt::StrongFocus);
  if (nullptr != m_pParent) { m_pParent->installEventFilter(this); }
  connect(m_pCloseButton, SIGNAL(clicked()), this, SLOT(disappear()));
  m_pTitle = new QLabel("title");
  m_pTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  m_pLayout->addWidget(m_pTitle, 0, 0);
  m_pCloseButton->setObjectName("pClose");
  m_pCloseButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  m_pLayout->addWidget(m_pCloseButton, 0, 1);
  m_pLayout->setMargin(0);
}

void OverlayWidget::setAutoDeleteOnClose(bool bAutoDelete)
{
  m_bAutoDeleteOnClose = bAutoDelete;
}

void OverlayWidget::addWidget(QWidget* pWidget, Qt::Alignment alignment)
{
  m_pLayout->addWidget(pWidget, m_pLayout->rowCount(), 0, 1, 2, alignment);
}

void OverlayWidget::setTitle(const QString& sText)
{
  m_pTitle->setText(sText);
}

void OverlayWidget::appear()
{
  QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "pos");
  pAnimation->setStartValue(QPoint(0, -window()->height()));
  pAnimation->setEndValue(QPoint(0,0));
  pAnimation->setDuration(200);
  pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  pAnimation->setEasingCurve(QEasingCurve::OutCubic);
  show();
  setFocus();
}

void OverlayWidget::disappear()
{
  QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "pos");
  pAnimation->setStartValue(pos());
  pAnimation->setEndValue(QPoint(0, -window()->height()));
  pAnimation->setDuration(200);
  pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  pAnimation->setEasingCurve(QEasingCurve::InCubic);

  if (m_bAutoDeleteOnClose)
  {
    connect(pAnimation, SIGNAL(finished()), this, SLOT(deleteLater()));
  }
}

void OverlayWidget::resizeEvent(QResizeEvent* pEvent)
{
  QFrame::resizeEvent(pEvent);

  if (nullptr != m_pParent)  { resize(m_pParent->size()); }
}

bool OverlayWidget::eventFilter(QObject* pObject, QEvent* pEvent)
{
  if (m_pParent == pObject)
  {
    switch (pEvent->type())
    {
    case QEvent::Resize:
      resize(m_pParent->size());
      break;
    case QEvent::KeyPress:
    {
      QKeyEvent* pKeyEvent = dynamic_cast<QKeyEvent*>(pEvent);
      if (nullptr != pKeyEvent &&
          Qt::Key_Escape == pKeyEvent->key())
      {
        disappear();
      }
    } break;
    default:
      break;
    }
  }
  return false;
}
