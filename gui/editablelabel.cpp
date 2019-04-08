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
  m_fnToDisplay = [](const QString& s){ return QString("\"%1\"").arg(s); };
}

void EditableLabel::mouseDoubleClickEvent(QMouseEvent* pMouseEvent)
{
  QLabel::mouseDoubleClickEvent(pMouseEvent);

  edit();
}

void EditableLabel::edit()
{
  QLineEdit* pEdit = new QLineEdit(this);
  pEdit->setText(m_sEditText.isEmpty() ? text() : m_sEditText);
  pEdit->selectAll();
  pEdit->setFocus();
  pEdit->resize(size());
  pEdit->show();
  connect(pEdit, SIGNAL(textChanged(QString)), this, SLOT(setEditText(QString)), Qt::UniqueConnection);
  connect(pEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()), Qt::UniqueConnection);
  connect(pEdit, SIGNAL(editingFinished()), pEdit, SLOT(deleteLater()), Qt::UniqueConnection);

  std::function<void(const QString&)> fn = std::bind(&EditableLabel::setText,
                                                     this, std::bind(m_fnToDisplay, std::placeholders::_1));
  connect(pEdit, &QLineEdit::textChanged, this, fn);
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

void EditableLabel::setEditText(const QString& sText)
{
  m_sEditText = sText;

  setText(m_fnToDisplay(sText));
}

QString EditableLabel::editText() const
{
  return m_sEditText;
}

QSize EditableLabel::sizeHint() const
{
  QFontMetrics fm(font());

  int iL = contentsRect().left() - rect().left();
  int iR = rect().right() - contentsRect().right();

  QRect r(iL, 0, (-1 == m_iSuggestedWidth ? contentsRect().width() : m_iSuggestedWidth) - iR - iL, 10000);

  int iFlags = alignment();
  if (wordWrap())  { iFlags |= Qt::TextWordWrap; }
  QRect boundingRect = style()->itemTextRect(fm, r, iFlags, true, text());


  return boundingRect.size();
}

void EditableLabel::resizeEvent(QResizeEvent* pEvent)
{
  QLabel::resizeEvent(pEvent);
//  updateGeometry();

  // only emit this if size really has changed!
//  if (pEvent->oldSize() != pEvent->size())
//  {
//    emit sizeChanged();
  //  }
}

bool EditableLabel::drawOutline() const
{
  return m_bDrawOutline;
}

void EditableLabel::setDrawOutline(bool bDraw)
{
  m_bDrawOutline = bDraw;
}

