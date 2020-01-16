#include "searchframe.h"
#include "ui_searchframe.h"

#include <QKeyEvent>
#include <QMainWindow>
#include <QStatusBar>
#include <QAction>


SearchFrame::SearchFrame(QWidget* pParent)
  : QFrame(pParent),
    ui(new Ui::SearchFrame)
{
  ui->setupUi(this);
  connect(ui->pText, &QLineEdit::textChanged, this, &SearchFrame::onTextChanged);
  connect(ui->pClose, &QPushButton::clicked, this, &SearchFrame::onExit);
  connect(ui->pFindNext, &QPushButton::clicked, this, &SearchFrame::findNext);
  connect(ui->pFindPrev, &QPushButton::clicked, this, &SearchFrame::findPrevious);
  connect(ui->pText, &QLineEdit::returnPressed, this, &SearchFrame::findNext);

  auto pEsc = new QAction("close");
  pEsc->setShortcut(Qt::Key_Escape);
  pEsc->setShortcutContext(Qt::WindowShortcut);
  connect(pEsc, &QAction::triggered, [&](){ onExit(); });
  addAction(pEsc);
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
  resize(width(), minimumSizeHint().height());
  setFocusOnLineEdit();
  emit searchTermChanged(ui->pText->text());
}

void SearchFrame::onPositionChanged(size_t currentPos, size_t total)
{
  if (0 == total)
  {
    showSearchInfo(tr("0 matches"));
  }
  else
  {
    showSearchInfo(tr("%1 of %2").arg(currentPos + 1).arg(total));
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
  showSearchInfo(QString());
}

void SearchFrame::showSearchInfo(const QString& sInfo)
{
  auto* pMainWindow = static_cast<QMainWindow*>(window());
  if (nullptr != pMainWindow && nullptr != pMainWindow->statusBar())
  {
    pMainWindow->statusBar()->showMessage(sInfo);
  }
}
