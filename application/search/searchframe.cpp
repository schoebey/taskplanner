#include "searchframe.h"
#include "ui_searchframe.h"

SearchFrame::SearchFrame(QWidget* pParent)
  : QFrame(pParent),
    ui(new Ui::SearchFrame)
{
  ui->setupUi(this);
  connect(ui->pText, &QLineEdit::textChanged, this, &SearchFrame::onTextChanged);
  connect(ui->pClose, &QToolButton::clicked, this, &SearchFrame::onExit);
}

void SearchFrame::onTextChanged(const QString& sText)
{
  emit searchTermChanged(sText);
}

void SearchFrame::onExit()
{

}
