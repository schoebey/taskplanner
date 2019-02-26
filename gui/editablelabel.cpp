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

    updateGeometry();
  }
}

QSize EditableLabel::sizeHint() const
{
  QFontMetrics fm(font());

  QRect r;

  if (wordWrap())
  {
    r = fm.boundingRect(QRect(0, 0, -1 == m_iSuggestedWidth ? width() : m_iSuggestedWidth, 10000),
                        Qt::AlignLeft | Qt::TextWordWrap, text());
  }
  else
  {
    r = fm.boundingRect(QRect(0, 0, 10000, fm.height()), 0, text());
  }

  return QSize(-1 == m_iSuggestedWidth ? width() : m_iSuggestedWidth, r.height());
}

void EditableLabel::paintEvent(QPaintEvent* pEvent)
{
  return QLabel::paintEvent(pEvent);
  QPainter painter(this);

  int iFlags = alignment();
  if (wordWrap())  { iFlags |= Qt::TextWordWrap; }
  else             { iFlags |= EditableLabel::eDrawShadowedText;}

  int align = QStyle::visualAlignment(Qt::LeftToRight, QFlag(alignment()));


  style()->drawItemText(&painter, contentsRect(), align, palette(), isEnabled(), text(), QPalette::Foreground);
}
