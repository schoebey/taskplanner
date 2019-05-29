#include "taskwidgetoverlay.h"
#include "taskwidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QDebug>

namespace
{
  std::pair<QColor, QColor> colorsFromMethod(HighlightingMethod method)
  {
    QColor borderColor(0,0,0,0);
    QColor highlightColor(0,0,0,0);

    // determine colors in descending ascending of importance
    // the last tested flag will overwrite all other colors
    if (method.testFlag(EHighlightMethod::eTimeTrackingActive))
    {
      borderColor = QColor(255, 200, 0);
      highlightColor = QColor(255, 200, 0, 50);
    }
    if (method.testFlag(EHighlightMethod::eFocus))
    {
      borderColor = QColor(140, 180, 255);
    }
    if (method.testFlag(EHighlightMethod::eHover))
    {
      highlightColor = QColor(255, 255, 255, 50);
    }
    if (method.testFlag(EHighlightMethod::eInsertPossible))
    {
      borderColor = Qt::red;
    }
    if (method.testFlag(EHighlightMethod::eValueAccepted))
    {
      borderColor = Qt::green;
      highlightColor = QColor(0, 255, 0, 100);
     }
    if (method.testFlag(EHighlightMethod::eValueRejected))
    {
      borderColor = Qt::red;
      highlightColor = QColor(255, 0, 0, 100);
     }

    return std::make_pair(borderColor, highlightColor);
  }
}

TaskWidgetOverlay::TaskWidgetOverlay(TaskWidget *parent)
  : QFrame(parent),
    m_pParent(parent)
{
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setFocusPolicy(Qt::NoFocus);
}

HighlightingMethod TaskWidgetOverlay::highlight() const
{
  return m_method;
}

void TaskWidgetOverlay::setHighlight(HighlightingMethod method)
{
  HighlightingMethod newlySetFlags = (m_method ^ method) & method;
  HighlightingMethod newlyRemovedFlags = (m_method ^ method) & m_method;


  // first, backup current colors
  QColor currentBorderColor(m_borderColor);
  QColor currentHighlightColor(m_highlightColor);


  // remove all non-permanent flags
  m_method = method & ~EHighlightMethod::eValueAccepted
                    & ~EHighlightMethod::eValueRejected;


  // then, determine the new colors based on the given method
  auto colors = colorsFromMethod(method);
  QColor newBorderColor(colors.first);
  QColor newHighlightColor(colors.second);


  // then, determine the animations based on the state changes of 'method'
  // (once for flags that have been set, once for flags that have been unset)
  if (newlySetFlags.testFlag(EHighlightMethod::eValueAccepted) ||
      newlySetFlags.testFlag(EHighlightMethod::eValueRejected))
  {
    // compute colors without the non-permanent flags
    // and use those as the new colors
    auto newColors = colorsFromMethod(m_method);
    if (newBorderColor.isValid())
    {
      setBorderColor(newBorderColor);
      setBorderColor(newColors.first, 1500);
    }

    if (newHighlightColor.isValid())
    {
      setHighlightColor(newHighlightColor);
      setHighlightColor(newColors.second, 1500);
    }
  }
  else
  {
    if (newBorderColor.isValid())
    {
      setBorderColor(newBorderColor, 250);
    }
  }


  // animate highlight color fade
  if (newlySetFlags.testFlag(EHighlightMethod::eTimeTrackingActive) ||
      newlySetFlags.testFlag(EHighlightMethod::eHover))
  {
    if (newHighlightColor.isValid())
    {
      QColor emphasis(newHighlightColor);
      emphasis.setAlpha(100);
      setHighlightColor(emphasis);
      setHighlightColor(newHighlightColor, 1500);
    }
  }
  else if (newlyRemovedFlags.testFlag(EHighlightMethod::eTimeTrackingActive) ||
           newlyRemovedFlags.testFlag(EHighlightMethod::eHover))
  {
    if (newHighlightColor.isValid())
    {
      setHighlightColor(newHighlightColor, 1500);
    }
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

QBrush TaskWidgetOverlay::background() const
{
  return m_backgroundBrush;
}

void TaskWidgetOverlay::setBackground(const QBrush& b)
{
  m_backgroundBrush = b;
}

QColor TaskWidgetOverlay::borderColor() const
{
 return m_borderColor;
}

void TaskWidgetOverlay::setBorderColor(const QColor& color)
{
  m_borderColor = color;
  update();
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

void TaskWidgetOverlay::paintEvent(QPaintEvent* /*pEvent*/)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  QColor borderColor(m_borderColor);
  QColor backgroundColor(m_highlightColor);

  static const int c_iBorderOffset = 5;
  QRectF rct(rect().adjusted(c_iBorderOffset, c_iBorderOffset, -c_iBorderOffset, -c_iBorderOffset));

  painter.setPen(Qt::NoPen);
  painter.setBrush(m_backgroundBrush);
  painter.drawRoundedRect(rct, 5, 5);

  painter.setPen(QPen(borderColor, 3));
  painter.setBrush(backgroundColor);
  painter.drawRoundedRect(rct, 5, 5);
}

