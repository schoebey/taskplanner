#ifndef TOOLBARINFODISPLAY_H
#define TOOLBARINFODISPLAY_H

#include <QFrame>

class QLabel;

class ToolBarInfoDisplay : public QFrame
{
  Q_OBJECT
public:
  explicit ToolBarInfoDisplay(QWidget *parent = nullptr);

signals:

public slots:

private:
  void UpdateInfo();
  void timerEvent(QTimerEvent* pEvent);

  QLabel* m_pLabel = nullptr;
};

#endif // TOOLBARINFODISPLAY_H
