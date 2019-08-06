#ifndef TOOLBARINFODISPLAY_H
#define TOOLBARINFODISPLAY_H

#include <QFrame>
#include <QJSEngine>

#include <thread>

class QLabel;

class ToolBarInfoDisplay : public QFrame
{
  Q_OBJECT
public:
  explicit ToolBarInfoDisplay(QWidget *parent = nullptr);

  bool startScript(const QString& sFileName);
  void stopScript();

  Q_PROPERTY(QString text READ text WRITE setText)
  QString text() const;
  void setText(const QString& sText);

private:
  void UpdateInfo();
  void timerEvent(QTimerEvent* pEvent);

  QLabel* m_pLabel = nullptr;
  QJSEngine m_engine;
  std::thread m_thread;
};

#endif // TOOLBARINFODISPLAY_H
