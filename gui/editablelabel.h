#ifndef EDITABLELABEL_H
#define EDITABLELABEL_H

#include <QLabel>




class EditableLabel : public QLabel
{
  Q_OBJECT
public:
  EditableLabel(QWidget* pParent);

  void mouseDoubleClickEvent(QMouseEvent* pMouseEvent) override;

  void edit();

  void suggestWidth(int iWidth);

  QSize sizeHint() const override;

  void resizeEvent(QResizeEvent*) override;

  Q_PROPERTY(bool drawOutline READ drawOutline WRITE setDrawOutline)
  bool drawOutline() const;
  void setDrawOutline(bool bDraw);

signals:
  void editingFinished();
  void sizeChanged();

private:
  int m_iSuggestedWidth = -1;
  bool m_bDrawOutline = false;
};

#endif // EDITABLELABEL_H
