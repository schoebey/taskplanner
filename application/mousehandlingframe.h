#ifndef MOUSEHANDLINGFRAME_H
#define MOUSEHANDLINGFRAME_H

#include <QFrame>

class MouseHandlingFrame : public QFrame
{
  Q_OBJECT
public:
  MouseHandlingFrame(QWidget* pParent = nullptr);
  ~MouseHandlingFrame() override;

signals:
  void mouseClicked(QPoint);
  void mouseDoubleClicked(QPoint);

protected:
  void mouseDoubleClickEvent(QMouseEvent*) override;
  void mousePressEvent(QMouseEvent* pEvent) override;
  void mouseReleaseEvent(QMouseEvent* pEvent) override;

private:
  QPoint m_mouseDownPt;
};

#endif // MOUSEHANDLINGFRAME_H
