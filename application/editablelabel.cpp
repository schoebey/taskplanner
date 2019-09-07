#include "editablelabel.h"
#include <QMouseEvent>
#include <QFocusEvent>
#include <QLineEdit>
#include <QStyle>
#include <QPainter>
#include <QStyleOption>


EditableLabel::EditableLabel(QWidget* pParent)
  : DecoratedLabel(pParent),
    m_pLineEdit(new QLineEdit(this)),
    m_iMinWidth(0)
{
  m_pLineEdit->hide();
  m_fnToDisplay = [](const QString& s){ return s; };
}

void EditableLabel::mouseDoubleClickEvent(QMouseEvent* pMouseEvent)
{
  QLabel::mouseDoubleClickEvent(pMouseEvent);

  edit();
}

void EditableLabel::edit()
{
  m_iMinWidth = sizeHint().width();
  setMinimumWidth(150);

  m_pLineEdit->setText(m_sEditText.isEmpty() ? text() : m_sEditText);
  m_pLineEdit->selectAll();
  m_pLineEdit->setFocus();
  m_pLineEdit->resize(size());
  m_pLineEdit->show();
  connect(m_pLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setEditText(QString)), Qt::UniqueConnection);
  connect(m_pLineEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()), Qt::UniqueConnection);
  connect(m_pLineEdit, SIGNAL(editingFinished()), m_pLineEdit, SLOT(hide()), Qt::UniqueConnection);

  std::function<void(const QString&)> fn = std::bind(&EditableLabel::setText,
                                                     this, std::bind(m_fnToDisplay, std::placeholders::_1));
  connect(m_pLineEdit, &QLineEdit::textChanged, this, fn);
}

void EditableLabel::setValue(const QString& sVal)
{
  setEditText(sVal);
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

void EditableLabel::setDisplayFunction(const std::function<QString(const QString&)> fnDisplay)
{
  m_fnToDisplay = fnDisplay;
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

void EditableLabel::updateDisplay()
{
  if (m_fnToDisplay)
  {
    setText(m_fnToDisplay(m_sEditText));
  }
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

  if (drawOutline())
  {
    boundingRect.adjust(-3, 0, 3, 0);
  }


  return boundingRect.size();
}

void EditableLabel::resizeEvent(QResizeEvent* pEvent)
{
  QLabel::resizeEvent(pEvent);
}

void EditableLabel::onEditingFinished()
{
  setMinimumWidth(m_iMinWidth);
  emit editingFinished();
  emit valueChanged(editText());
}

