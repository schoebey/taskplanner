#ifndef EDITABLELABEL_H
#define EDITABLELABEL_H

#include <QLabel>




class EditableLabel : public QLabel
{
  Q_OBJECT
public:
  enum EditableLabelFlags
  {
    eDrawShadowedText = 0x10000000,
  };


  EditableLabel(QWidget* pParent);

  void mouseDoubleClickEvent(QMouseEvent* pMouseEvent);

  void edit();

  void suggestWidth(int iWidth);

  QSize sizeHint() const override;

  void paintEvent(QPaintEvent* pEvent) override;
signals:
  void editingFinished();

private:
  int m_iSuggestedWidth = -1;
};

#endif // EDITABLELABEL_H
