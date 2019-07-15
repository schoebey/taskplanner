#ifndef DECORATEDLABEL_H
#define DECORATEDLABEL_H

#include <QLabel>

class DecoratedLabel : public QLabel
{
public:
  DecoratedLabel(QWidget* pParent = nullptr);
  DecoratedLabel(const QString& sText, QWidget* pParent = nullptr);

  Q_PROPERTY(bool drawOutline READ drawOutline WRITE setDrawOutline)
  bool drawOutline() const;
  void setDrawOutline(bool bDraw);

  Q_PROPERTY(bool drawShadow READ drawShadow WRITE setDrawShadow)
  bool drawShadow() const;
  void setDrawShadow(bool bDraw);

private:
  bool m_bDrawOutline = false;
  bool m_bDrawShadow = false;
};

#endif // DECORATEDLABEL_H
