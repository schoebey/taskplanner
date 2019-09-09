#include "datetimelabel.h"
#include "conversion.h"

DateTimeLabel::DateTimeLabel(QWidget* pParent)
  : EditableLabel(pParent)
{
  setFocusPolicy(Qt::NoFocus);
  setAlignment(Qt::AlignCenter);

  setDisplayFunction(conversion::fancy::dateToString);
  connect(&m_updateTimer, &QTimer::timeout, [this]()
  {
    bool bStatus(false);
    QDateTime dt = conversion::fromString<QDateTime>(editText(), bStatus);
    if (bStatus)
    {
      updateDisplay();

      // restart the timer with a closer timeout as the due date approaches
      qint64 iSecsTo = QDateTime::currentDateTime().msecsTo(dt) / 1000;
      if (0 <= iSecsTo)
      {
        static const int c_iMaxTimeoutMs = 300000;
        int iTimeoutMs = std::min<int>(c_iMaxTimeoutMs, static_cast<int>(iSecsTo * 1000));

        if (iTimeoutMs <= 10000)
        {
          iTimeoutMs = 1000;
        }
        else if (iTimeoutMs <= 30000)
        {
          iTimeoutMs = 5000;
        }
        else if (iTimeoutMs <= 60000)
        {
          iTimeoutMs = 15000;
        }
        else if (iTimeoutMs <= c_iMaxTimeoutMs)
        {
          iTimeoutMs = c_iMaxTimeoutMs / 2;
        }

        // signal the outside world that the property of this editor needs attention
        bool bAttentionNeeded = iSecsTo <= 3600;
        if (bAttentionNeeded)
        {
          emit attentionNeeded();
        }

        m_updateTimer.start(iTimeoutMs);
      }
    }
  });

  m_updateTimer.setSingleShot(true);
  m_updateTimer.start(1000);
}

void DateTimeLabel::onEditingFinishedImpl()
{
  m_updateTimer.start(1000);
}
