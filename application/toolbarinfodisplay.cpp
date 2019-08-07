#include "toolbarinfodisplay.h"

#include <QPainter>
#include <QGridLayout>
#include <QLabel>
#include <QDate>
#include <QFile>
#include <QTextStream>


CommObject::CommObject()
{
}

CommObject::~CommObject()
{
}

QString CommObject::text() const
{
  return m_sText;
}

void CommObject::setText(const QString& sText)
{
  if (m_sText != sText)
  {
    m_sText = sText;
    emit textChanged(m_sText);
  }
}

void CommObject::quit()
{
  m_bQuit = true;
}

bool CommObject::shouldQuit() const
{
  return m_bQuit;
}

void CommObject::sleep(int iMs)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(iMs));
}


ToolBarInfoDisplay::ToolBarInfoDisplay(QWidget *parent)
  : QFrame(parent)
{
  QGridLayout* pLayout = new QGridLayout(this);
  setLayout(pLayout);

  m_pLabel = new QLabel(this);
  pLayout->addWidget(m_pLabel, 0, 0);

  m_iTimer = startTimer(60000);
  UpdateInfo();
}

ToolBarInfoDisplay::~ToolBarInfoDisplay()
{
  stopScript();
}

bool ToolBarInfoDisplay::startScript(const QString& sFileName,
                                     QString* psErrorMessage)
{
  m_sScriptPath.clear();

  QFile scriptFile(sFileName);
  if (!scriptFile.open(QIODevice::ReadOnly))
  {
    return false;
  }

  m_sScriptPath = sFileName;

  QTextStream stream(&scriptFile);
  QString contents = stream.readAll();
  scriptFile.close();

  // stop a potentially already running thread
  stopScript();

  CommObject* pObj = new CommObject();
  connect(this, &ToolBarInfoDisplay::quit, pObj, &CommObject::quit);
  connect(pObj, &CommObject::textChanged, this, &ToolBarInfoDisplay::setText);
  QJSValue scriptObject = m_engine.newQObject(pObj);
  m_engine.globalObject().setProperty("display", scriptObject);
  QJSValue fn = m_engine.evaluate(contents, sFileName);
  if (fn.isError())
  {
    if (nullptr != psErrorMessage)
    {
      *psErrorMessage = tr("JavaScript error: %1").arg(fn.toString());
    }

    return false;
  }

  auto fnEval = [this, fn]()
  {
    auto fnNonConst = fn;
    auto res = fnNonConst.call();
    if (res.isError())
    {
      emit showError(tr("JavaScript error: %1").arg(res.toString()));
    }
  };

  killTimer(m_iTimer);
  m_thread = std::thread{fnEval};
  m_thread.detach();

  return true;
}

void ToolBarInfoDisplay::stopScript()
{
  // signal the script that it should stop
  emit quit();
  if (m_thread.joinable())
  {
    m_thread.join();
  }
}

QString ToolBarInfoDisplay::text() const
{
  return m_pLabel->text();
}

void ToolBarInfoDisplay::setText(const QString& sText)
{
  m_pLabel->setText(sText);
}

QString ToolBarInfoDisplay::scriptPath() const
{
  return m_sScriptPath;
}

void ToolBarInfoDisplay::UpdateInfo()
{
  m_pLabel->setText(tr("CW %1").arg(QDate::currentDate().weekNumber()));
}

void ToolBarInfoDisplay::timerEvent(QTimerEvent* /*pEvent*/)
{
  UpdateInfo();
}


