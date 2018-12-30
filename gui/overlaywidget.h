#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QFrame>

class QGridLayout;
class QPushButton;
class OverlayWidget : public QFrame
{
  Q_OBJECT
public:
  explicit OverlayWidget(QWidget *parent = nullptr);

  void setAutoDeleteOnClose(bool bAutoDelete);

  void addWidget(QWidget* pWidget);

  void keyPressEvent(QKeyEvent* pEvent) override;

signals:

public slots:
  void appear();
  void disappear();

private:
  void resizeEvent(QResizeEvent* pEvent) override;
  bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private:
  bool m_bAutoDeleteOnClose = false;
  QWidget* m_pParent = nullptr;
  QGridLayout* m_pLayout = nullptr;
  QPushButton* m_pCloseButton = nullptr;
};

#endif // OVERLAYWIDGET_H
