#include "linkwidget.h"
#include "ui_linkwidget.h"

#include <QPainter>
#include <QFileIconProvider>

#include <cmath>

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
    setProperty("isFile", true);
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
    setProperty("isFile", false);
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    ui->pIcon->setPixmap(pm);
    ui->pLabel->setText(m_link.toString());
  }



  //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  resize(32, 32);
  setMinimumSize(32, 32);
}

LinkWidget::~LinkWidget()
{

}

void LinkWidget::setConstrainLabelToSize(bool bConstrain)
{
  m_bConstrainLabelToSize = bConstrain;
  if (bConstrain)
  {
//    ui->pLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
    ui->pLabel->setVisible(false);
  }
  else
  {
    ui->pLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    ui->pLabel->setVisible(true);
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
    QSize s(m_pToolTip->sizeHint());
    m_pToolTip->resize(s);
    QPoint ptTl = ui->pIcon->mapTo(m_pToolTip->parentWidget(), ui->pIcon->rect().topLeft());
    QPointF ptCenter = ptTl + QPointF(ui->pIcon->rect().width() / 2.,
                                      ui->pIcon->rect().height() / 2.);

    QPointF ptTooltipTl = m_pToolTip->ui->pIcon->mapTo(m_pToolTip, m_pToolTip->ui->pIcon->rect().topLeft());
    QPointF ptTooltipCenter = ptTooltipTl + QPointF(m_pToolTip->ui->pIcon->rect().width() / 2.,
                                                    m_pToolTip->ui->pIcon->rect().height() / 2.);
    QPointF offset = ptTooltipCenter;// - ptTooltipTl;
    m_pToolTip->move(ptCenter.x() - floor(offset.x()), ptCenter.y() - floor(offset.y()));
  }
}

void LinkWidget::leaveEvent(QEvent* pEvent)
{
  delete m_pToolTip;
  m_pToolTip = nullptr;
}

void LinkWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
//  QPainter painter(this);
//  painter.setPen(Qt::green);
//  painter.setBrush(Qt::red);

//  QRect r(rect());
//  painter.drawRect(r);
}
