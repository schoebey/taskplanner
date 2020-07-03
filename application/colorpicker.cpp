#include "colorpicker.h"
#include "colorspacewidget.h"

#include <QGridLayout>
#include <QSlider>

ColorPicker::ColorPicker(QWidget* pParent)
  : QWidget(pParent)
{
  QGridLayout* pLayout = new QGridLayout(this);

  m_pCsw = new ColorSpaceWidget(this);
  pLayout->addWidget(m_pCsw, 0, 0);
  connect(m_pCsw, &ColorSpaceWidget::colorChanged,
          this, &ColorPicker::colorChanged);

  m_pSatSlider = new QSlider(this);
  m_pSatSlider->setOrientation(Qt::Vertical);
  m_pSatSlider->setRange(0, 255);
  pLayout->addWidget(m_pSatSlider, 0, 1);
  connect(m_pSatSlider, &QSlider::valueChanged,
          this, &ColorPicker::onSaturationChanged);
}

QColor ColorPicker::color() const
{
  return m_pCsw->color();
}

void ColorPicker::setColor(const QColor& c)
{
  m_pSatSlider->setValue(c.hslSaturation());
  m_pCsw->setColor(c);
}

void ColorPicker::onSaturationChanged(int iSat)
{
  m_pCsw->setSaturation(iSat);
}
