#ifndef TOOLBARINFODISPLAY_H
#define TOOLBARINFODISPLAY_H

#include <QFrame>
#include <QJSEngine>

#include <thread>

class QLabel;

class CommObject : public QObject
{
  Q_OBJECT
public:
  CommObject();
  ~CommObject();


  Q_PROPERTY(QString text READ text WRITE setText)
  QString text() const;
  void setText(const QString& sText);

  Q_PROPERTY(bool quit READ shouldQuit)
  bool shouldQuit() const;
  void quit();

  Q_INVOKABLE void sleep(int iMs);

signals:
  void textChanged(const QString& sText);

private:
  QString m_sText;
  bool m_bQuit = false;
};

class ToolBarInfoDisplay : public QFrame
{
  Q_OBJECT
public:
  explicit ToolBarInfoDisplay(QWidget *parent = nullptr);
  ~ToolBarInfoDisplay() override;

  bool startScript(const QString& sFileName, QString* psErrorMessage = nullptr);
  void stopScript();

  Q_PROPERTY(QString text READ text WRITE setText)
  QString text() const;
  void setText(const QString& sText);

  QString scriptPath() const;

signals:
  void showError(const QString& sError);
  void quit();

private:
  void UpdateInfo();
  void timerEvent(QTimerEvent* pEvent) override;


  QLabel* m_pLabel = nullptr;
  int m_iTimer = -1;
  QJSEngine m_engine;
  std::thread m_thread;
  QString m_sScriptPath;
};

#endif // TOOLBARINFODISPLAY_H
