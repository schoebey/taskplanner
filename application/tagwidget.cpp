#include "tagwidget.h"
#include "styleExtension.h"

#include <QPainter>
#include <QStyleOption>


TagWidget::TagWidget(const QString& sText, QWidget* pParent)
  : QLabel(sText, pParent)
{

}

void TagWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  QStyleOptionFrame opt;
  opt.initFrom(this);

  QPainter painter(this);
  style()->drawControl(static_cast<QStyle::ControlElement>(CE_TagWidget), &opt, &painter, this);
}
