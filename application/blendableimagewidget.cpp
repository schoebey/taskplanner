#include "blendableimagewidget.h"

#include <QPainter>
#include <QPropertyAnimation>

BlendableImageWidget::BlendableImageWidget(QWidget* pParent)
  : QFrame(pParent)
{
}

double BlendableImageWidget::backgroundImageBlendFactor() const
{
  return m_dBackgroundImageBlendFactor;
}

void BlendableImageWidget::setBackgroundImageBlendFactor(double dFactor)
{
  m_dBackgroundImageBlendFactor = dFactor;
  update();
}

void BlendableImageWidget::setBackgroundImage(const QImage& image)
{
  m_backgroundImage[1] = m_backgroundImage[0];
  m_backgroundImage[0] = image;
  m_dBackgroundImageBlendFactor = 1;

  QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "backgroundImageBlendFactor");
  static const int c_iDuration = 1000;
  pAnimation->setDuration(c_iDuration);
  pAnimation->setStartValue(1);
  pAnimation->setEndValue(0);
  pAnimation->setEasingCurve(QEasingCurve::Linear);
  pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void BlendableImageWidget::resizeEvent(QResizeEvent* pEvent)
{
  QFrame::resizeEvent(pEvent);

  m_cache = QPixmap();
}

void BlendableImageWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  bool bRefreshCache = m_cache.isNull() || !qFuzzyIsNull(m_dBackgroundImageBlendFactor);
  if (bRefreshCache)
  {
    m_cache = QPixmap(width(), height());
    m_cache.fill(Qt::transparent);
    QPainter painter(&m_cache);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QRectF rct(contentsRect());

    painter.save();
    QPainterPath path;
    path.addRoundedRect(rct, 7, 7);
    painter.setClipPath(path);
    QPointF offset(pos().x()/5, pos().y()/5);
    QBrush b(m_backgroundImage[0]);
    painter.setPen(Qt::NoPen);
    painter.setBrush(b);
    painter.drawRect(rct);


    QBrush f(m_backgroundImage[1]);
    painter.setOpacity(m_dBackgroundImageBlendFactor);
    painter.setBrush(f);
    painter.drawRect(rct);

    painter.restore();
  }

  QPainter painter(this);
  painter.drawPixmap(0, 0, m_cache);
}
