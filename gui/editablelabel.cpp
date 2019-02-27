#include "editablelabel.h"
#include <QMouseEvent>
#include <QFocusEvent>
#include <QLineEdit>
#include <QStyle>
#include <QPainter>
#include <QStyleOption>


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
  connect(pEdit, SIGNAL(textChanged(QString)), this, SLOT(setText(QString)));
  connect(pEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
  connect(pEdit, SIGNAL(editingFinished()), pEdit, SLOT(deleteLater()));
}

void EditableLabel::suggestWidth(int iWidth)
{
  if (m_iSuggestedWidth != iWidth)
  {
    m_iSuggestedWidth = iWidth;

    setMinimumHeight(sizeHint().height());

    updateGeometry();
  }
}

QSize EditableLabel::sizeHint() const
{
  QFontMetrics fm(font());

  int iL = contentsRect().left() - rect().left() + 2;
  int iR = rect().right() - contentsRect().right() + 2;

  QRect r(iL, 0, (-1 == m_iSuggestedWidth ? contentsRect().width() : m_iSuggestedWidth) - iR, 10000);

  int iFlags = alignment();
  if (wordWrap())  { iFlags |= Qt::TextWordWrap; }
  QRect boundingRect = style()->itemTextRect(fm, r, iFlags, true, text());


  return boundingRect.size();
}

void EditableLabel::resizeEvent(QResizeEvent*)
{
  updateGeometry();
}

void EditableLabel::paintEvent(QPaintEvent* pEvent)
{
  QLabel::paintEvent(pEvent);

//  int iFlags = alignment();
//  if (wordWrap())  { iFlags |= Qt::TextWordWrap; }

//  //int align = QStyle::visualAlignment(Qt::LeftToRight, QFlag(alignment()));


//  QRect crect = contentsRect();
//  style()->drawItemText(&painter, crect, iFlags, palette(), isEnabled(), text(), QPalette::Foreground);
}
