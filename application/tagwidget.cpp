#include "tagwidget.h"
#include "styleExtension.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>

#include <cmath>

QStyleOptionTagWidget::QStyleOptionTagWidget()
  : QStyleOption(1, customStyleOptions::SO_TagWidget)
{

}

template<> Draggable<TagWidget>* Draggable<TagWidget>::m_pDraggingInstance = nullptr;

TagWidget::TagWidget(const QString& sText, QWidget* pParent)
  : QWidget(pParent),
    m_sText(sText),
    m_color(QColor(255, 210, 20))
{
  setAttribute(Qt::WA_StyledBackground, true);
}

TagWidget::TagWidget(const TagWidget& other)
  : QWidget(other.parentWidget())
{
  setText(other.text());
  setColor(other.color());
  resize(other.size());
}

TagWidget::~TagWidget()
{

}

void TagWidget::setText(const QString& sText)
{
  if (m_sText != sText)
  {
    m_sText = sText;
    emit textChanged(m_sText);
  }
}

QString TagWidget::text() const
{
  return m_sText;
}

void TagWidget::setColor(const QColor& c)
{
  if (c != m_color)
  {
    m_color = c;
    emit colorChanged(m_color);
  }
}

QColor TagWidget::color() const
{
  return m_color;
}

void TagWidget::showEvent(QShowEvent* pEvent)
{

}

void TagWidget::hideEvent(QHideEvent* pEvent)
{

}

QSize TagWidget::sizeHint() const
{
  int iHeaderSize = style()->pixelMetric(customPixelMetrics::PM_TagHeader);
  int iBorderSize = style()->pixelMetric(customPixelMetrics::PM_TagBorder);

  QFontMetrics fm(font());
  int iWidth = fm.width(text());
  int iHeight = fm.height();

  return QSize(iWidth, iHeight) + QSize(iHeaderSize, 0) + QSize(2 * iBorderSize, 2 * iBorderSize);
}

void TagWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  QStyleOptionTagWidget opt;
  opt.init(this);

  opt.sText = text();
  opt.color = m_color;

  QPainter painter(this);
  style()->drawControl(customControlElements::CE_TagWidget, &opt, &painter, this);
}


