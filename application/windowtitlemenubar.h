#ifndef TITLEBARMENU_H
#define TITLEBARMENU_H

#include <QLabel>
#include <QMenuBar>

class QPushButton;
class WindowTitleMenuBar : public QMenuBar
{
  Q_OBJECT
public:
  explicit WindowTitleMenuBar(QWidget* pParent = nullptr);

  void addWidget(QWidget* pWidget);

private slots:
  void toggleMaximize(bool bMaximize);
  void maximize();
  void minimize();
  void restore();
  void close();
  void updateWidgets();

private:
  void mousePressEvent(QMouseEvent* pEvent) override;
  void mouseDoubleClickEvent(QMouseEvent *pEvent) override;
  void mouseMoveEvent(QMouseEvent* pEvent) override;
  void mouseReleaseEvent(QMouseEvent* pEvent) override;
  void resizeEvent(QResizeEvent *pEvent) override;
  bool eventFilter(QObject *pObject, QEvent *pEvent) override;


  bool m_bDraggingWindow = false;
  QPoint m_mouseDownPoint;
  QLabel* m_pTitle = nullptr;
  QFrame* m_pRightButtonBox = nullptr;
  QPushButton* m_pMaximizeButton = nullptr;
};

#endif // TITLEBARMENU_H
