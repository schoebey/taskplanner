#ifndef EDITABLELABEL_H
#define EDITABLELABEL_H

#include <QLabel>



class QLineEdit;
class EditableLabel : public QLabel
{
  Q_OBJECT
public:
  EditableLabel(QWidget* pParent);

  void mouseDoubleClickEvent(QMouseEvent* pMouseEvent) override;

  void suggestWidth(int iWidth);

  QSize sizeHint() const override;

  void resizeEvent(QResizeEvent*) override;

  Q_PROPERTY(bool drawOutline READ drawOutline WRITE setDrawOutline)
  bool drawOutline() const;
  void setDrawOutline(bool bDraw);

  void setDisplayFunction(const std::function<QString(const QString&)> fnDisplay);

public slots:
  void edit();

  void setEditText(const QString& sText);
  QString editText() const;

signals:
  void editingFinished();
  void sizeChanged();

private slots:
  void onEditingFinished();

private:
  QLineEdit* m_pLineEdit = nullptr;
  int m_iSuggestedWidth = -1;
  bool m_bDrawOutline = false;
  QString m_sEditText;
  std::function<QString(const QString&)> m_fnToDisplay;
  int m_iMinWidth;
};

#endif // EDITABLELABEL_H
