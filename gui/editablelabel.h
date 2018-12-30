#ifndef EDITABLELABEL_H
#define EDITABLELABEL_H

#include <QLabel>



class EditableLabel : public QLabel
{
  Q_OBJECT
public:
  EditableLabel(QWidget* pParent);

  void mouseDoubleClickEvent(QMouseEvent* pMouseEvent);

  void edit();

signals:
  void editingFinished();
};

#endif // EDITABLELABEL_H
