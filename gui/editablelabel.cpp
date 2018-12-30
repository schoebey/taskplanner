#include "editablelabel.h"
#include <QMouseEvent>
#include <QFocusEvent>
#include <QLineEdit>

EditableLabel::EditableLabel(QWidget* pParent)
  : QLabel(pParent)
{

}

void EditableLabel::mouseDoubleClickEvent(QMouseEvent* pMouseEvent)
{
  QLabel::mouseDoubleClickEvent(pMouseEvent);

  edit();
}

void EditableLabel::edit()
{
  QLineEdit* pEdit = new QLineEdit(this);
  pEdit->setText(text());
  pEdit->selectAll();
  pEdit->setFocus();
  pEdit->resize(size());
  pEdit->show();
  connect(pEdit, SIGNAL(textChanged(QString)), this, SLOT(setText(QString)));
  connect(pEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
  connect(pEdit, SIGNAL(editingFinished()), pEdit, SLOT(deleteLater()));
}
