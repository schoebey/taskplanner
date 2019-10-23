#ifndef DATETIMELABEL_H
#define DATETIMELABEL_H

#include "editablelabel.h"

#include <QTimer>

class QCalendarWidget;
class DateTimeLabel : public EditableLabel
{
  Q_OBJECT
public:
  using EditableLabel::valueChanged;

  DateTimeLabel(QWidget* pParent);

  void edit() override;

signals:
  void attentionNeeded();

private:
  void onEditingFinishedImpl() override;
  bool eventFilter(QObject* pObj, QEvent* pEvent);
  void rejectAndCloseEditor();
  void onTextChanged(const QString& sText);

private:
  QTimer m_updateTimer;
  QWidget* m_pEditor;
  QLineEdit* m_pDateTimeLineEdit;
  QCalendarWidget* m_pCalendarWidget;

};

#endif // DATETIMELABEL_H
