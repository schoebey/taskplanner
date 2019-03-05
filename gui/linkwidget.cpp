#include "linkwidget.h"
#include "ui_linkwidget.h"

#include <QPainter>
#include <QFileIconProvider>

LinkWidget::LinkWidget(const QUrl& link)
  : ui(new Ui::LinkWidget),
    m_link(link)
{
  ui->setupUi(this);
  setConstrainLabelToSize(true);


  QString s(m_link.toLocalFile());
  QFileInfo info(s);
  if (info.exists())
  {
    QFileIconProvider p;
    QIcon icon = p.icon(info);
    if (!icon.isNull())
    {
      ui->pIcon->setPixmap(icon.pixmap(QSize(32, 32)));
    }

    if (!info.fileName().isEmpty())
    {
      ui->pLabel->setText(info.fileName());
    }
    else
    {
      auto elements = info.filePath().split("/");
      if (2 <= elements.size())
      {
        ui->pLabel->setText(elements[elements.size() - 2]);
      }
    }
  }
  else
  {
    ui->pLabel->setText(m_link.toString());
  }


  //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  resize(32, 32);
  setMinimumSize(32, 32);
  setToolTip(link.toString());
}

LinkWidget::~LinkWidget()
{

}

void LinkWidget::setConstrainLabelToSize(bool bConstrain)
{
  m_bConstrainLabelToSize = bConstrain;
  if (bConstrain)
  {
    ui->pLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
  }
  else
  {
    ui->pLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  }
}

bool LinkWidget::constrainLabelToSize() const
{
  return m_bConstrainLabelToSize;
}

void LinkWidget::showToolTip()
{
}

void LinkWidget::enterEvent(QEvent* pEvent)
{
  if (nullptr == m_pToolTip)
  {
    m_pToolTip = new LinkWidget(m_link);
    m_pToolTip->setConstrainLabelToSize(false);
    m_pToolTip->setParent(window());
    m_pToolTip->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_pToolTip->setFocusPolicy(Qt::NoFocus);
    m_pToolTip->show();
    m_pToolTip->resize(m_pToolTip->sizeHint());
  }
}

void LinkWidget::leaveEvent(QEvent* pEvent)
{
  delete m_pToolTip;
  m_pToolTip = nullptr;
}

void LinkWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  QPainter painter(this);
  painter.setPen(Qt::green);
  painter.setBrush(Qt::red);

  QRect r(rect());
  painter.drawRect(r);
}
