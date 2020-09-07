#include "tagwidget.h"
#include "styleExtension.h"
#include "editablelabel.h"
#include "colorpicker.h"

#include <QGridLayout>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QWidgetAction>

#include <cmath>


TagWidget::TagWidget(const QString& sText, QWidget* pParent)
  : QFrame(pParent),
    m_color(QColor(255, 210, 20))
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
  connect(pLabel, &EditableLabel::textChanged, this, &TagWidget::setText);
  setText(sText);
  setEditable(false);
}

TagWidget::TagWidget(const TagWidget& other)
  : QFrame(other.parentWidget())
{
  QGridLayout* pLayout = new QGridLayout();
  pLayout->setSpacing(0);
  pLayout->setMargin(0);
  setLayout(pLayout);
  EditableLabel* pLabel = new EditableLabel(this);
  pLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_pLabel = pLabel;
  pLayout->addWidget(pLabel);
  connect(pLabel, &EditableLabel::textChanged, this, &TagWidget::setText);
  setText(other.text());
  setColor(other.color());
  resize(other.size());
  setEditable(false);
}

TagWidget::~TagWidget()
{

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


