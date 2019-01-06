#include "overlaywidget.h"
#include <QLayout>
#include <QPushButton>
#include <QEvent>
#include <QLabel>
#include <QStyle>
#include <QPropertyAnimation>
#include <QPaintEvent>

OverlayWidget::OverlayWidget(QWidget *parent)
  : QFrame(parent->window()),
    m_pParent(parent),
    m_pLayout(new QGridLayout(this)),
    m_pCloseButton(new QPushButton("X", this))
{
  setFocusPolicy(Qt::StrongFocus);
  m_pParent->installEventFilter(this);
  connect(m_pCloseButton, SIGNAL(clicked()), this, SLOT(disappear()));
  QLabel* pTitle = new QLabel("title");
  pTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  m_pLayout->addWidget(pTitle, 0, 0);
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

void OverlayWidget::appear()
{
  QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "pos");
  pAnimation->setStartValue(QPoint(0, window()->height()));
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
  pAnimation->setEndValue(QPoint(0,window()->height()));
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

  resize(m_pParent->size());
}

bool OverlayWidget::eventFilter(QObject* pObject, QEvent* pEvent)
{
  if (m_pParent == pObject && QEvent::Resize == pEvent->type())
  {
    resize(m_pParent->size());
  }
  return false;
}

void OverlayWidget::keyPressEvent(QKeyEvent* pEvent)
{
  switch (pEvent->key())
  {
  case Qt::Key_Escape:
    disappear();
  default:
    break;
  }
}
