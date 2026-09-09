#include "reminderdialog.h"

#include <QComboBox>
#include <QSpinBox>
#include <QTimeEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDate>

ReminderDialog::ReminderDialog(QWidget* pParent) :
  QDialog(pParent)
{
  setWindowTitle(tr("Reminder"));

  m_pRepeatMode = new QComboBox(this);
  m_pRepeatMode->addItem(tr("Recurring"), int(EReminderRepeatMode::Recurring));
  m_pRepeatMode->addItem(tr("Single shot"), int(EReminderRepeatMode::SingleShot));

  m_pIntervalUnit = new QComboBox(this);
  m_pIntervalUnit->addItem(tr("Hours"), int(EReminderIntervalUnit::Hours));
  m_pIntervalUnit->addItem(tr("Days"), int(EReminderIntervalUnit::Days));

  m_pIntervalCount = new QSpinBox(this);
  m_pIntervalCount->setRange(0, 10000);

  m_pTriggerTime = new QTimeEdit(this);
  m_pTriggerTime->setDisplayFormat("hh:mm:ss");

  auto* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto* pLayout = new QFormLayout(this);
  pLayout->addRow(tr("Repeat mode:"), m_pRepeatMode);
  pLayout->addRow(tr("Interval unit:"), m_pIntervalUnit);
  pLayout->addRow(tr("Interval count:"), m_pIntervalCount);
  pLayout->addRow(tr("Trigger time:"), m_pTriggerTime);
  pLayout->addRow(pButtonBox);
}

SReminder ReminderDialog::reminder() const
{
  SReminder reminder;

  reminder.repeatMode = static_cast<EReminderRepeatMode>(m_pRepeatMode->currentData().toInt());
  reminder.intervalUnit = static_cast<EReminderIntervalUnit>(m_pIntervalUnit->currentData().toInt());
  reminder.iIntervalCount = m_pIntervalCount->value();
  reminder.triggerTime = m_pTriggerTime->time();

  if (EReminderRepeatMode::SingleShot == reminder.repeatMode)
  {
    reminder.dueDateTime = QDateTime(QDate::currentDate(), reminder.triggerTime);
  }

  return reminder;
}

void ReminderDialog::setReminder(const SReminder& reminder)
{
  int iRepeatModeIdx = m_pRepeatMode->findData(int(reminder.repeatMode));
  if (-1 != iRepeatModeIdx) { m_pRepeatMode->setCurrentIndex(iRepeatModeIdx); }

  int iUnitIdx = m_pIntervalUnit->findData(int(reminder.intervalUnit));
  if (-1 != iUnitIdx) { m_pIntervalUnit->setCurrentIndex(iUnitIdx); }

  m_pIntervalCount->setValue(reminder.iIntervalCount);
  m_pTriggerTime->setTime(reminder.triggerTime);
}
