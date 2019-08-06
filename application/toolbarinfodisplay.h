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

  bool startScript(const QString& sFileName, QString* psErrorMessage = nullptr);
  void stopScript();

  Q_PROPERTY(QString text READ text WRITE setText)
  QString text() const;
  void setText(const QString& sText);

  QString scriptPath() const;

signals:
  void showError(const QString& sError);

private:
  void UpdateInfo();
  void timerEvent(QTimerEvent* pEvent);

  QLabel* m_pLabel = nullptr;
  QJSEngine m_engine;
  std::thread m_thread;
  QString m_sScriptPath;
};

#endif // TOOLBARINFODISPLAY_H
