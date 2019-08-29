#include "searchframe.h"
#include "ui_searchframe.h"

#include <QKeyEvent>

SearchFrame::SearchFrame(QWidget* pParent)
  : QFrame(pParent),
    ui(new Ui::SearchFrame)
{
  ui->setupUi(this);
  connect(ui->pText, &QLineEdit::textChanged, this, &SearchFrame::onTextChanged);
  connect(ui->pClose, &QToolButton::clicked, this, &SearchFrame::onExit);
  connect(ui->pFindNext, &QPushButton::clicked, this, &SearchFrame::findNext);
  connect(ui->pFindPrev, &QPushButton::clicked, this, &SearchFrame::findPrevious);
  connect(ui->pText, &QLineEdit::returnPressed, this, &SearchFrame::findNext);
}

void SearchFrame::keyPressEvent(QKeyEvent* pEvent)
{
  switch (pEvent->key())
  {
  case Qt::Key_Escape:
    onExit();
    break;
  default:
    break;
  }
}

void SearchFrame::onTextChanged(const QString& sText)
{
  emit searchTermChanged(sText);
}

void SearchFrame::setFocusOnLineEdit()
{
  ui->pText->setFocus();
}

void SearchFrame::onOpen()
{
  setFocusOnLineEdit();
  emit searchTermChanged(ui->pText->text());
}

void SearchFrame::onPositionChanged(size_t currentPos, size_t total)
{
  if (0 == total)
  {
    ui->pHitsInfo->setText(tr("0 matches"));
  }
  else
  {
    ui->pHitsInfo->setText(tr("%1 of %2").arg(currentPos + 1).arg(total));
    if (!isVisible())
    {
      setVisible(true);
      onOpen();
    }
  }
}

void SearchFrame::onExit()
{
  emit searchTermChanged(QString());
  hide();
}
