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

  void paintEvent(QPaintEvent* pEvent) override;
  void resizeEvent(QResizeEvent*);
signals:
  void editingFinished();

private:
  int m_iSuggestedWidth = -1;
};

#endif // EDITABLELABEL_H
