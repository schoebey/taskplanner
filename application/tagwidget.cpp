#include "tagwidget.h"
#include "styleExtension.h"
#include "editablelabel.h"
#include "colorpicker.h"

#include <QGridLayout>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QWidgetAction>
#include <QStyleOptionFrameV3>
#include <QDebug>

#include <cmath>


TagWidget::TagWidget(tag_id id, const QString& sText, QWidget* pParent)
  : QFrame(pParent),
    m_id(id)
{
  setAttribute(Qt::WA_StyledBackground, true);
  QGridLayout* pLayout = new QGridLayout();
  pLayout->setSpacing(0);
  pLayout->setMargin(0);
  setLayout(pLayout);
  EditableLabel* pLabel = new EditableLabel(this);
  pLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_pLabel = pLabel;
  pLayout->addWidget(pLabel);
  connect(pLabel, &EditableLabel::textChanged, this, &TagWidget::textChanged);
  setText(sText);
  setEditable(false);
  setColor(QColor(255, 210, 20));
}

TagWidget::~TagWidget()
{

}

tag_id TagWidget::id() const
{
  return m_id;
}

void TagWidget::setText(const QString& sText)
{
  if (m_pLabel->text() != sText)
  {
    m_pLabel->setText(sText);
    emit textChanged(sText);
  }
}

QString TagWidget::text() const
{
  return m_pLabel->text();
}

void TagWidget::setColor(const QColor& c)
{
  if (c != m_color)
  {
    m_color = c;

    QPalette pal(m_pLabel->palette());
    bool bBrightText = qGray(m_color.rgb()) <= 120;
    pal.setColor(QPalette::Text, bBrightText ? Qt::white : Qt::black);
    m_pLabel->setPalette(pal);
    m_pLabel->setProperty("brightText", bBrightText);

    style()->unpolish(m_pLabel);
    style()->polish(m_pLabel);

    style()->unpolish(this);
    style()->polish(this);

    emit colorChanged(m_color);
  }
}

QColor TagWidget::color() const
{
  return m_color;
}

void TagWidget::setEditable(bool bEditable)
{
  if (bEditable ^ editable())
  {
    m_pLabel->setEditable(bEditable);
    emit editabilityChanged();

    if (bEditable && nullptr == m_pColorPickerAction)
    {
      QWidgetAction* pAction = new QWidgetAction(this);
      auto pColorWidget = new ColorPicker();
      pColorWidget->setMinimumSize(130, 100);
      pColorWidget->setColor(color());
      connect(pColorWidget, &ColorPicker::colorChanged, this, &TagWidget::setColor);
      connect(this, &TagWidget::colorChanged, pColorWidget, &ColorPicker::setColor);
      pAction->setDefaultWidget(pColorWidget);
      addAction(new QAction("blub"));
      addAction(pAction);
      addAction(new QAction("blub bli"));
      addAction(new QAction("blub bla"));
      setContextMenuPolicy(Qt::ActionsContextMenu);
      m_pColorPickerAction = pAction;
    }
  }
}

bool TagWidget::editable() const
{
  return m_pLabel->editable();
}

void TagWidget::paintEvent(QPaintEvent* pEvent)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::black);
  painter.setBrush(m_color);
  painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);

//  QStyleOptionFrame opt;
//  opt.initFrom(this);
//  for (int i = 0; i < QPalette::NColorRoles; ++i)
//  {
//    opt.palette.setColor(static_cast<QPalette::ColorRole>(i), Qt::yellow);
//  }
//  painter.setBrush(Qt::yellow);
//  style()->drawPrimitive(QStyle::PE_Frame, &opt, &painter, this);

//  QFrame::paintEvent(pEvent);
}
