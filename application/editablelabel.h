#ifndef EDITABLELABEL_H
#define EDITABLELABEL_H

#include "decoratedlabel.h"
#include <functional>


class QLineEdit;
class EditableLabel : public DecoratedLabel
{
  Q_OBJECT
public:
  EditableLabel(QWidget* pParent);

  void mouseDoubleClickEvent(QMouseEvent* pMouseEvent) override;

  void suggestWidth(int iWidth);

  QSize sizeHint() const override;

  void resizeEvent(QResizeEvent*) override;

  void setDisplayFunction(const std::function<QString(const QString&)> fnDisplay);

public slots:
  void edit();

  void setEditText(const QString& sText);
  QString editText() const;

  void updateDisplay();

signals:
  void editingFinished();
  void sizeChanged();
  void textChanged(const QString&);

private slots:
  void onEditingFinished();

private:
  QLineEdit* m_pLineEdit = nullptr;
  int m_iSuggestedWidth = -1;
  QString m_sEditText;
  std::function<QString(const QString&)> m_fnToDisplay;
  int m_iMinWidth;
};

#endif // EDITABLELABEL_H
