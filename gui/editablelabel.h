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

  void suggestWidth(int iWidth);

  QSize sizeHint() const override;

signals:
  void editingFinished();

private:
  int m_iSuggestedWidth = -1;
};

#endif // EDITABLELABEL_H
