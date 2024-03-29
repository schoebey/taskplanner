#include "taskwidgetoverlay.h"
#include "taskwidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QDebug>

namespace
{
  struct SHighlightProperties
  {
    QColor borderColor;
    QColor highlightColor;
    int iFadeTimeMs = 1500;
  };

  SHighlightProperties colorsFromMethod(HighlightingMethod method)
  {
    SHighlightProperties props;
    props.borderColor = QColor(0,0,0,0);
    props.highlightColor = QColor(0,0,0,0);

    // determine colors in descending ascending of importance
    // the last tested flag will overwrite all other colors
    if (method.testFlag(EHighlightMethod::eTimeTrackingActive))
    {
      props.borderColor = QColor(255, 200, 0);
      props.highlightColor = QColor(255, 200, 0, 50);
    }
    if (method.testFlag(EHighlightMethod::eFocus))
    {
      props.borderColor = QColor(140, 180, 255);
    }
    if (method.testFlag(EHighlightMethod::eHover))
    {
      double dBlendFactor = 0.75;
      props.highlightColor = colorsFromMethod(method & ~EHighlightMethod::eHover).highlightColor;
      props.highlightColor = QColor(props.highlightColor.red() * (1 - dBlendFactor) + 255 * dBlendFactor,
                                    props.highlightColor.green() * (1 - dBlendFactor) + 255 * dBlendFactor,
                                    props.highlightColor.blue() * (1 - dBlendFactor) + 255 * dBlendFactor,
                                    150);
    }
    if (method.testFlag(EHighlightMethod::eInsertPossible))
    {
      props.borderColor = Qt::red;
    }
    if (method.testFlag(EHighlightMethod::eValueAccepted))
    {
      props.borderColor = Qt::green;
      props.highlightColor = QColor(0, 255, 0, 100);
     }
    if (method.testFlag(EHighlightMethod::eValueRejected))
    {
      props.borderColor = Qt::red;
      props.highlightColor = QColor(255, 0, 0, 100);
     }
    if (method.testFlag(EHighlightMethod::eSearchResult))
    {
      props.borderColor = QColor(255, 200, 0, 100);
      props.highlightColor = QColor(255, 200, 0, 200);
      props.iFadeTimeMs = 10;

      if (method.testFlag(EHighlightMethod::eFocus))
      {
        props.borderColor.setAlpha(255);
        props.highlightColor = QColor(100, 130, 255, 200);
        props.highlightColor.setAlpha(255);
      }
    }

    return props;
  }
}

TaskWidgetOverlay::TaskWidgetOverlay(QWidget* pParent)
  : FloatingWidget(pParent)
{
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
  newlyRemovedFlags & ~EHighlightMethod::eValueAccepted
                    & ~EHighlightMethod::eValueRejected;


  // then, determine the new colors based on the given method
  auto colors = colorsFromMethod(method);
  QColor newBorderColor(colors.borderColor);
  QColor newHighlightColor(colors.highlightColor);


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
      setBorderColor(newColors.borderColor, colors.iFadeTimeMs);
    }

    if (newHighlightColor.isValid())
    {
      setHighlightColor(newHighlightColor);
      setHighlightColor(newColors.highlightColor, colors.iFadeTimeMs);
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
  if (0 != newlySetFlags &&
      !newlySetFlags.testFlag(EHighlightMethod::eValueAccepted) &&
      !newlySetFlags.testFlag(EHighlightMethod::eValueRejected))
  {
    if (newHighlightColor.isValid())
    {
      QColor emphasis(newHighlightColor);
      emphasis.setAlpha(100);
      setHighlightColor(emphasis);
      setHighlightColor(newHighlightColor, colors.iFadeTimeMs);
    }
  }
  else if (0 != newlyRemovedFlags)
  {
    if (newHighlightColor.isValid())
    {
      setHighlightColor(newHighlightColor, colors.iFadeTimeMs);
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

  QRectF rct(rect());

  painter.setPen(Qt::NoPen);
  painter.setBrush(m_backgroundBrush);
  painter.drawRoundedRect(rct, 5, 5);

  painter.setPen(QPen(borderColor, 3));
  painter.setBrush(backgroundColor);
  painter.drawRoundedRect(rct, 5, 5);
}
