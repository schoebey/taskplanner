#include "linkwidget.h"

#include <QPainter>
#include <QFileIconProvider>

LinkWidget::LinkWidget(const QUrl& link)
  : m_link(link)
{
  //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  resize(32, 32);
  setMinimumSize(32, 32);
  setToolTip(link.toString());
}

LinkWidget::~LinkWidget()
{

}

void LinkWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  QPainter painter(this);
  painter.setPen(Qt::green);
  painter.setBrush(Qt::red);

  QRect r(rect());
  painter.drawRect(r);

  QString s(m_link.toLocalFile());
  QFileInfo info(s);
  if (info.exists())
  {
    QFileIconProvider p;
    QIcon icon = p.icon(info);
    if (!icon.isNull())
    {
      painter.drawPixmap(0, 0, icon.pixmap(size()));
    }
  }
}
