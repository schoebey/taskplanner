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

  Q_PROPERTY(bool editable READ editable WRITE setEditable NOTIFY editabilityChanged)
  void setEditable(bool bEditable);
  bool editable() const;

  QString previousText() const;

public slots:
  void edit();
  void cancel();
  void closeEditor();

  void setEditText(const QString& sText);
  QString editText() const;

  void updateDisplay();

signals:
  void editingFinished();
  void sizeChanged();
  void textChanged(const QString&);
  void editabilityChanged();

protected:
  bool eventFilter(QObject* pWatched, QEvent* pEvent) override;

private slots:
  void onEditingFinished();

private:
  QLineEdit* m_pLineEdit = nullptr;
  int m_iSuggestedWidth = -1;
  QString m_sEditText;
  std::function<QString(const QString&)> m_fnToDisplay;
  int m_iMinWidth;
  bool m_bEditable = true;
  QString m_sPrevText;
};

#endif // EDITABLELABEL_H
