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

  /*!
   * \brief corrects the coordinates based on window size and magnetic distance to stick to the nearest screen border
   * \param targetPos targeted position of the window
   * \param iMagneticDistance distance at which the window gets 'attracted' to the border
   * \return corrected coordinate to move the window to in order to 'stick' it to the border
   */
  QPoint stickToScreenBorders(const QPoint& targetPos, int iMagneticDistance);


  bool m_bDraggingWindow = false;
  QPoint m_mouseDownPoint;
  QLabel* m_pTitle = nullptr;
  QFrame* m_pRightButtonBox = nullptr;
  QPushButton* m_pMaximizeButton = nullptr;
};

#endif // TITLEBARMENU_H
