#ifndef WIDGETRESIZER_H
#define WIDGETRESIZER_H

#include <QFrame>

class WidgetResizer : public QFrame
{
  Q_OBJECT

public:
  enum EPosition
  {
    eTopLeft,
    eTop,
    eTopRight,
    eRight,
    eBottomRight,
    eBottom,
    eBottomLeft,
    eLeft,
    eCenter,
    eNone
  };


  WidgetResizer(QWidget* pParent, EPosition anchor);

  void mousePressEvent(QMouseEvent* pEvent) override;

  void mouseDoubleClickEvent(QMouseEvent* pEvent) override;

  void mouseReleaseEvent(QMouseEvent* pEvent) override;

  void mouseMoveEvent(QMouseEvent* pEvent) override;

  bool eventFilter(QObject* pObj, QEvent* pEvent) override;

protected slots:
  void update(bool bVisible);

private:
  QWidget*  m_pParent;
  bool      m_bMouseDown;
  QPoint    m_mouseDownPt;
  EPosition m_anchor;
};


#endif // WIDGETRESIZER_H
