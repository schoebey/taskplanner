#include "decoratedlabel.h"

DecoratedLabel::DecoratedLabel(QWidget* pParent)
  : QLabel(pParent)
{
}

DecoratedLabel::DecoratedLabel(const QString& sText, QWidget* pParent)
  : QLabel(sText, pParent)
{
}

bool DecoratedLabel::drawOutline() const
{
  return m_bDrawOutline;
}

void DecoratedLabel::setDrawOutline(bool bDraw)
{
  m_bDrawOutline = bDraw;
}

bool DecoratedLabel::drawShadow() const
{
  return m_bDrawShadow;
}

void DecoratedLabel::setDrawShadow(bool bDraw)
{
  m_bDrawShadow = bDraw;
}
