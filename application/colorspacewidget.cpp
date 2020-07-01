#include "colorspacewidget.h"

#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QRadialGradient>

#include <cmath>

namespace
{
  static const double c_dPi = 3.1415;
  static const double c_dTau = 2 * c_dPi;

  QPointF coordsFromColor(const QColor& col,
                          const QRect& rect)
  {
    double h = col.hslHueF();
    double l = col.lightnessF();

    QPointF pt(-1, 0);
    QTransform t;
    t.rotateRadians((h - 0.5) * c_dTau);
    pt = t.map(pt);
    pt *= (1 - l);

    double dRadius = std::min<double>(rect.width(), rect.height());
    pt *= dRadius;
    pt.setX(pt.x() + rect.width() / 2);
    pt.setY(pt.y() + rect.height() / 2);
    return pt;
  }

  QColor colorFromCoords(const QPointF& pt,
                         const QRect& rect,
                         double dSat = 1.)
  {
    QColor c;
    QPointF ptCenter(rect.width() / 2., rect.height() / 2.);
    double dRadius = std::min<double>(ptCenter.x(), ptCenter.y());
    QPointF vec(ptCenter.x() - pt.x(), ptCenter.y() - pt.y());
    vec /= dRadius;
    double dLen = std::sqrt(std::pow(vec.x(), 2) + std::pow(vec.y(), 2));
    if (dLen <= 1)
    {
      double h = atan2(vec.y(), vec.x()) / c_dTau + 0.5;
      h = std::max<double>(std::min<double>(h, 1), 0);
      c = QColor::fromHslF(h, dSat, 0.5 + (1 - dLen) / 2);
    }

    return c;
  }
}

ColorSpaceWidget::ColorSpaceWidget()
{
  setColor(Qt::yellow);
}


void ColorSpaceWidget::resizeEvent(QResizeEvent* pEvent)
{
  m_buffer = QImage(pEvent->size(), QImage::Format_ARGB32);
  QPainter painter(&m_buffer);
  painter.fillRect(m_buffer.rect(), Qt::transparent);

  for (int y = 0; y < m_buffer.height(); ++y)
  {
    for (int x = 0; x < m_buffer.width(); ++x)
    {
      QColor c = colorFromCoords(QPointF(x, y), m_buffer.rect(), m_dSaturation);
      if (c.isValid())
      {
        m_buffer.setPixel(x, y, c.rgba());
      }
    }
  }
}

void ColorSpaceWidget::mousePressEvent(QMouseEvent* pEvent)
{
  QColor c = colorFromCoords(pEvent->pos(), m_buffer.rect());
  setColor(c);
}

void ColorSpaceWidget::mouseMoveEvent(QMouseEvent* pEvent)
{
  if (pEvent->button() == Qt::LeftButton)
  {
    QColor c = colorFromCoords(pEvent->pos(), m_buffer.rect());
    setColor(c);
  }
}

void ColorSpaceWidget::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.drawImage(0, 0, m_buffer);


  if (m_currentCol.isValid())
  {
    QPointF pt = coordsFromColor(m_currentCol, m_buffer.rect());

    static const double c_dRadius = 2;
    painter.setPen(Qt::black);
    painter.drawEllipse(pt, c_dRadius, c_dRadius);
  }
}

QColor ColorSpaceWidget::color() const
{
  return m_currentCol;
}

void ColorSpaceWidget::setColor(const QColor& c)
{
  if (m_currentCol != c)
  {
    m_currentCol = c;
    update();
    emit colorChanged(m_currentCol);
  }
}

double ColorSpaceWidget::saturation() const
{
  return m_dSaturation;
}

void ColorSpaceWidget::setSaturation(double dSaturation)
{
  if (qFuzzyIsNull(dSaturation) ^ qFuzzyIsNull(m_dSaturation) ||
      !qFuzzyCompare(dSaturation, m_dSaturation))
  {
    m_dSaturation = dSaturation;
    emit saturationChanged(m_dSaturation);
  }
}
