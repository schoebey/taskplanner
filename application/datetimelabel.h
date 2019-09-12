#ifndef DATETIMELABEL_H
#define DATETIMELABEL_H

#include "editablelabel.h"

#include <QTimer>

class DateTimeLabel : public EditableLabel
{
  Q_OBJECT
public:
  using EditableLabel::valueChanged;

  DateTimeLabel(QWidget* pParent);

signals:
  void attentionNeeded();

private:
  void onEditingFinishedImpl() override;

private:
  QTimer m_updateTimer;
};

#endif // DATETIMELABEL_H
