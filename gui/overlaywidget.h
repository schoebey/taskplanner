#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QFrame>

class QGridLayout;
class QPushButton;
class QLabel;
class OverlayWidget : public QFrame
{
  Q_OBJECT
public:
  explicit OverlayWidget(QWidget *parent = nullptr);

  void setAutoDeleteOnClose(bool bAutoDelete);

  void addWidget(QWidget* pWidget, Qt::Alignment alignment = 0);

  void keyPressEvent(QKeyEvent* pEvent) override;

  void setTitle(const QString& sText);
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
  QLabel* m_pTitle = nullptr;
};

#endif // OVERLAYWIDGET_H
