#include "editablelabel.h"
#include <QMouseEvent>
#include <QFocusEvent>
#include <QLineEdit>
#include <QStyle>
#include <QPainter>
#include <QStyleOption>
#include <QKeyEvent>


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
  if (m_bEditable)
  {
    m_iMinWidth = sizeHint().width();
    setMinimumWidth(150);

    m_sPrevText = text();

    m_pLineEdit->setText(m_sEditText.isEmpty() ? text() : m_sEditText);
    m_pLineEdit->selectAll();
    m_pLineEdit->setFocus();
    m_pLineEdit->resize(size());
    m_pLineEdit->show();
    connect(m_pLineEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()), Qt::UniqueConnection);


    m_pLineEdit->installEventFilter(this);
  }
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
  setEditText(m_pLineEdit->text());
  emit textChanged(m_pLineEdit->text());

  setText(m_fnToDisplay(m_pLineEdit->text()));

  closeEditor();
}

void EditableLabel::cancel()
{
  m_pLineEdit->setText(m_sEditText.isEmpty() ? text() : m_sEditText);
  closeEditor();
}

void EditableLabel::closeEditor()
{
  m_pLineEdit->hide();
  m_pLineEdit->removeEventFilter(this);
  setMinimumWidth(m_iMinWidth);
  emit editingFinished();
}

void EditableLabel::setEditable(bool bEditable)
{
  if (m_bEditable ^ bEditable)
  {
    m_bEditable = bEditable;
    emit editabilityChanged();
  }
}

bool EditableLabel::editable() const
{
  return m_bEditable;
}

QString EditableLabel::previousText() const
{
  return m_sPrevText;
}

bool EditableLabel::eventFilter(QObject* pWatched, QEvent* pEvent)
{
  if (m_pLineEdit == pWatched)
  {
    switch (pEvent->type())
    {
    case QEvent::KeyPress:
    {
      QKeyEvent* pKeyEvent = dynamic_cast<QKeyEvent*>(pEvent);
      if (nullptr != pKeyEvent &&
          Qt::Key_Escape == pKeyEvent->key())
      {
        cancel();
      }
    }break;
    default:
      break;
    }
  }

  return false;
}
