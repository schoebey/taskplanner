#ifndef REMINDERDIALOG_H
#define REMINDERDIALOG_H

#include "reminder.h"

#include <QDialog>

class QComboBox;
class QSpinBox;
class QTimeEdit;

class ReminderDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ReminderDialog(QWidget* pParent = nullptr);

  SReminder reminder() const;
  void setReminder(const SReminder& reminder);

private:
  QComboBox* m_pRepeatMode;
  QComboBox* m_pIntervalUnit;
  QSpinBox* m_pIntervalCount;
  QTimeEdit* m_pTriggerTime;
};

#endif // REMINDERDIALOG_H
