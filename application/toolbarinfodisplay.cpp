#include "toolbarinfodisplay.h"

#include <QPainter>
#include <QGridLayout>
#include <QLabel>
#include <QDate>

ToolBarInfoDisplay::ToolBarInfoDisplay(QWidget *parent)
  : QFrame(parent)
{
  setMinimumSize(20, 20);
  QGridLayout* pLayout = new QGridLayout(this);
  setLayout(pLayout);

  m_pLabel = new QLabel(this);
  pLayout->addWidget(m_pLabel, 0, 0);

  startTimer(60000);
  UpdateInfo();
}

void ToolBarInfoDisplay::UpdateInfo()
{
  m_pLabel->setText(QString("KW %1").arg(QDate::currentDate().weekNumber()));
}

void ToolBarInfoDisplay::timerEvent(QTimerEvent* /*pEvent*/)
{
  UpdateInfo();
}
