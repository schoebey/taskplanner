#include "taskwidgetoverlay.h"
#include "taskwidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPropertyAnimation>

TaskWidgetOverlay::TaskWidgetOverlay(TaskWidget *parent)
  : QWidget(parent),
    m_pParent(parent)
{
  setAttribute(Qt::WA_TransparentForMouseEvents);
}

HighlightingMethod TaskWidgetOverlay::highlight() const
{
  return m_method;
}

void TaskWidgetOverlay::setHighlight(HighlightingMethod method)
{
  m_method = method;

  if (m_method.testFlag(EHighlightMethod::eValueAccepted))
  {
    QColor currentColor(m_borderColor);
    QColor currentHighlightColor(m_highlightColor);
    setBorderColor(Qt::green);
    setBorderColor(currentColor, 1500);
    setHighlightColor(QColor(0, 255, 0, 100));
    setHighlightColor(currentHighlightColor, 1500);
   }
  if (m_method.testFlag(EHighlightMethod::eValueRejected))
  {
    QColor currentColor(m_borderColor);
    QColor currentHighlightColor(m_highlightColor);
    setBorderColor(Qt::red);
    setBorderColor(currentColor, 1500);
    setHighlightColor(QColor(255, 0, 0, 100));
    setHighlightColor(currentHighlightColor, 1500);
   }
  if (m_method.testFlag(EHighlightMethod::eTimeTrackingActive))
  {
    setBorderColor(QColor(255, 200, 0), 500);
    setHighlightColor(QColor(255, 200, 0, 100));
    setHighlightColor(QColor(255, 200, 0, 50), 1500);
  }
  if (m_method.testFlag(EHighlightMethod::eFocus))
  {
    QColor currentColor(m_borderColor);
    QColor currentHighlightColor(m_highlightColor);
    setBorderColor(Qt::green);
    setBorderColor(currentColor, 1500);
   }
  if (m_method.testFlag(EHighlightMethod::eNoHighlight))
  {
    QColor b(m_borderColor);
    b.setAlpha(0);
    setBorderColor(b, 500);

    QColor c(m_highlightColor);
    c.setAlpha(0);
    setHighlightColor(c, 500);
  }
}

QColor TaskWidgetOverlay::highlightColor() const
{
  return m_highlightColor;
}

void TaskWidgetOverlay::setHighlightColor(const QColor& color)
{
  m_highlightColor = color;
  update();
}

void TaskWidgetOverlay::setHighlightColor(const QColor& color, int iMsecs)
{
  QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "highlightColor");
  pAnimation->setDuration(iMsecs);
  pAnimation->setStartValue(highlightColor());
  pAnimation->setEndValue(color);
  pAnimation->setEasingCurve(QEasingCurve::OutQuad);
  pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

QColor TaskWidgetOverlay::borderColor() const
{
 return m_borderColor;
}

void TaskWidgetOverlay::setBorderColor(const QColor& color)
{
  m_borderColor = color;
}

void TaskWidgetOverlay::setBorderColor(const QColor& color, int iMsecs)
{
  QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "borderColor");
  pAnimation->setDuration(iMsecs);
  pAnimation->setStartValue(borderColor());
  pAnimation->setEndValue(color);
  pAnimation->setEasingCurve(QEasingCurve::OutQuad);
  pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void TaskWidgetOverlay::paintEvent(QPaintEvent* pEvent)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  QColor borderColor(m_borderColor);
  QColor backgroundColor(m_highlightColor);

  painter.setPen(QPen(borderColor, 3));
  painter.setBrush(backgroundColor);

  static const int c_iBorderOffset = 5;
  QRectF rct(rect().adjusted(c_iBorderOffset, c_iBorderOffset, -c_iBorderOffset, -c_iBorderOffset));
  painter.drawRoundedRect(rct, 5, 5);
}


