#include "datetimelabel.h"
#include "conversion.h"

#include <QCalendarWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QApplication>
#include <QKeyEvent>
#include <QMenu>

DateTimeLabel::DateTimeLabel(QWidget* pParent)
  : EditableLabel(pParent),
    m_pEditor(new QFrame()),
    m_pDateTimeLineEdit(new QLineEdit(this)),
    m_pCalendarWidget(new QCalendarWidget(this))
{
  QVBoxLayout* pLayout = new QVBoxLayout();
  pLayout->addWidget(m_pDateTimeLineEdit);
  pLayout->addWidget(m_pCalendarWidget);
  pLayout->setSpacing(3);
  pLayout->setContentsMargins(3, 3, 3, 3);
  m_pEditor->setWindowFlag(Qt::Popup);
  m_pEditor->setObjectName("pDateTimeEditor");
  m_pEditor->setLayout(pLayout);
  m_pEditor->hide();

  /*
   * expected behavior: return press: accept value and close editor
   * escape: reject value and close editor
   * click on date: accept value and close editor
   * click outside the editor: reject value and close editor
   * click anywhere else within the editor: keep editor open
   */
  connect(m_pDateTimeLineEdit, &QLineEdit::returnPressed, this, &DateTimeLabel::onEditingFinishedImpl);
  connect(m_pDateTimeLineEdit, &QLineEdit::textChanged, this, &DateTimeLabel::onTextChanged);
  connect(m_pCalendarWidget, &QCalendarWidget::clicked, this, &DateTimeLabel::onEditingFinishedImpl);

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

DateTimeLabel::~DateTimeLabel()
{
  delete m_pEditor;
}

bool DateTimeLabel::eventFilter(QObject* pObj, QEvent* pEvent)
{
  if (!m_pEditor->isVisible())
  {
    return false;
  }

  switch (pEvent->type())
  {
  case QEvent::MouseButtonPress:
    if (pObj->isWidgetType())
    {
      QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
      const QWidget* pWidget = qobject_cast<QWidget*>(pObj);
      const QMenu* pMenu = qobject_cast<QMenu*>(pObj);

      if (!m_pEditor->rect().contains(m_pEditor->mapFromGlobal(pMouseEvent->globalPos())) &&
          (nullptr == pMenu || !m_pEditor->isAncestorOf(pMenu->parentWidget())))
      {
        rejectAndCloseEditor();
      }

//      if (m_pEditor != pWidget &&
//          !m_pEditor->isAncestorOf(pWidget) &&
//          (nullptr == pMenu || !m_pEditor->isAncestorOf(pMenu->parentWidget())))
//      {
//        rejectAndCloseEditor();
//      }
    }
    break;
  case QEvent::KeyPress:
  {
    QKeyEvent* pKeyEvent = dynamic_cast<QKeyEvent*>(pEvent);
    if (nullptr != pKeyEvent &&
        Qt::Key_Escape == pKeyEvent->key())
    {
      rejectAndCloseEditor();
    }
   } break;
  default:
    break;
  }

  return false;
}

void DateTimeLabel::edit()
{
  qApp->installEventFilter(this);

  // TODO: show the popup in a popup window so that it can spill over the window borders...

  bool bStatus(false);
  QDateTime dt = conversion::fromString<QDateTime>(editText(), bStatus);
  m_pCalendarWidget->setSelectedDate(bStatus ? dt.date() : QDate());
  m_pDateTimeLineEdit->setText(editText());
  m_pDateTimeLineEdit->selectAll();
  m_pDateTimeLineEdit->setFocus();

  m_pEditor->resize(m_pEditor->minimumSizeHint());
  m_pEditor->move(mapToGlobal(QPoint(0, height())));
  m_pEditor->show();
}

void DateTimeLabel::onTextChanged(const QString& sText)
{
  // update the calendar to reflect the new date
  bool bStatus(false);
  QDateTime dt = conversion::fromString<QDateTime>(sText, bStatus);
  if (bStatus)
  {
    m_pCalendarWidget->setSelectedDate(dt.date());
  }
}

void DateTimeLabel::rejectAndCloseEditor()
{
  qApp->removeEventFilter(this);
  m_pEditor->hide();
}

void DateTimeLabel::onEditingFinishedImpl()
{
  qApp->removeEventFilter(this);
  m_updateTimer.start(1000);
  m_pEditor->hide();

  QDate d = m_pCalendarWidget->selectedDate();

  bool bStatus(false);
  QString sEditText = m_pDateTimeLineEdit->text();

  QDateTime dt = QDateTime::currentDateTime();
  if (!sEditText.isEmpty())
  {
    dt = conversion::fromString<QDateTime>(sEditText, bStatus);
    if (bStatus)
    {
      dt.setDate(d);
    }
  }
  else
  {
    dt.setTime(QTime::currentTime());
  }

  setEditText(conversion::toString(dt));
}
