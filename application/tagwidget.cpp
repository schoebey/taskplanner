#include "tagwidget.h"
#include "styleExtension.h"
#include "draggable.h"
#include "editablelabel.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QVector3D>

#include <cmath>

namespace
{
  static const double c_dPi = 3.1415;

  double length(const QPointF& pt)
  {
    return sqrt(pow(pt.x(), 2) + pow(pt.y(), 2));
  }

  double dotproduct(const QPointF& pt1, const QPointF& pt2)
  {
    return QPointF::dotProduct(pt1, pt2);
//    return pt1.x() * pt2.x() + pt1.y() * pt2.y();
  }

  double sign(const QPointF& pt1, const QPointF& pt2)
  {
    QVector3D crossProd = QVector3D::crossProduct(QVector3D(pt1.x(), pt1.y(), 0),
                                                  QVector3D(pt2.x(), pt2.y(), 0));
    return crossProd.z() >= 0 ? -1 : 1;
  }
}

QStyleOptionTagWidget::QStyleOptionTagWidget()
  : QStyleOption(1, customStyleOptions::SO_TagWidget)
{

}

template<> Draggable<TagWidget>* Draggable<TagWidget>::m_pDraggingInstance = nullptr;

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
}

TagWidget::TagWidget(const TagWidget& other)
  : QFrame(other.parentWidget()),
    m_origin(other.m_origin),
    m_size(other.m_size),
    m_expandedSize(other.m_expandedSize)
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

void TagWidget::setAngle(double dAngleRad)
{
  m_dAngleRad = dAngleRad;
  update();
}

double TagWidget::angle() const
{
  return m_dAngleRad;
}

void TagWidget::setOrigin(QPoint pt)
{
  m_origin = pt;
}

QPoint TagWidget::origin() const
{
  return m_origin;
}

void TagWidget::setEditable(bool bEditable)
{
  m_pLabel->setEditable(bEditable);
}

bool TagWidget::editable() const
{
  return m_pLabel->editable();
}

void TagWidget::resizeEvent(QResizeEvent* pEvent)
{
  QFrame::resizeEvent(pEvent);


  if (nullptr != parentWidget())
  {
    m_size = pEvent->size();

    int iDimension = 2 * sqrt(pow(rect().width(), 2) + pow(rect().height(), 2));
    m_expandedSize = QSize(iDimension, iDimension);
  }
}

void TagWidget::timerEvent(QTimerEvent *event)
{
  stepSimulation();
}

void TagWidget::stepSimulation()
{
  // simulate pendulum motion based on rect and center of rotation
  // if mouse is moved, the angle is altered based on the distance and direction of movement
  static const double c_dDecaySpeed = 4;

  // TODO: compute period T based on compound pendulum formula
  double T = 1;

  // calculate time in motion
  auto now = std::chrono::steady_clock::now();
  auto deltaT = now - m_startTime;
  double dMsecs = std::chrono::duration_cast<std::chrono::milliseconds>(deltaT).count();
  double dSecs = dMsecs / 1000;

  // calculate max. amplitude by factoring in the amplitude decay speed
  double dStartAngleRad = m_dSimulationStartAngleRad - c_dPi / 2;
  double dAmpRad = dStartAngleRad / (1 + dSecs * c_dDecaySpeed);

  // calculate the fraction in the current iteration
  double dPhase = fmod(dSecs, T) / T * 2 * c_dPi;

  // and finally calculate the angle of the pendulum
  double dAngleRad = cos(dPhase) * dAmpRad;

  setAngle(dAngleRad + c_dPi / 2);
}

void TagWidget::startSimulation()
{
  m_startTime = std::chrono::steady_clock::now();

  if (-1 != m_iTimerId)  { killTimer(m_iTimerId); }
  m_iTimerId = startTimer(20);
}

void TagWidget::stopSimulation()
{
  if (-1 != m_iTimerId)
  {
    killTimer(m_iTimerId);
    m_iTimerId = -1;
  }
}

bool TagWidget::event(QEvent* pEevent)
{
  switch (pEevent->type())
  {
  case QEvent::ParentChange:
    if (nullptr == parent())
    {
      if (m_expandedSize.isValid() &&
          m_expandedSize.width() > 0 && m_expandedSize.height() > 0)
      {
        resize(m_expandedSize);

        // start simulation
        startSimulation();
      }

      m_pLabel->setVisible(false);
    }
    else
    {
      if (m_size.isValid() &&
          m_size.width() > 0 && m_size.height() > 0)
      {
        resize(m_size);
      }

      m_pLabel->setVisible(true);

      stopSimulation();
    }
    break;
  }

  return QFrame::event(pEevent);
}

void TagWidget::mousePressEvent(QMouseEvent* pEvent)
{
  setOrigin(pEvent->pos());
}

void TagWidget::mouseMoveEvent(QMouseEvent* pEvent)
{
  if (nullptr == parentWidget())
  {
    stopSimulation();

    if (m_image.isNull())
    {
      QWidget* pRef = static_cast<QWidget*>(property("reference").value<void*>());
      if (nullptr != pRef) {
        m_image = pRef->grab().toImage();
      }
    }

    QPointF motionVector = rect().center() - pEvent->pos();
    qDebug() << "motion:" << motionVector;

    // normalize motion vector magnitude relative to
    // the distance between rotation center and mass center
    double dCurrentAngleRad = angle();
    QPointF axis(m_size.width() / 2, 0);
    QTransform t;
    t.rotate(dCurrentAngleRad * 180 / c_dPi);
    axis = t.map(axis);

    double dMotionVectorForce = std::max<double>(0, std::min<double>(1, length(motionVector) / length(axis)));
    double dDotProd = dotproduct(motionVector, axis);
    double dSign = sign(motionVector, axis);
    double dAngularDistanceRad = dSign * std::acos(dDotProd / (length(motionVector) * length(axis)));

    dCurrentAngleRad += dAngularDistanceRad * dMotionVectorForce;
    m_dSimulationStartAngleRad = dCurrentAngleRad;

    startSimulation();
    stepSimulation();
  }
}

QSize TagWidget::sizeHint() const
{
  int iHeaderSize = style()->pixelMetric(customPixelMetrics::PM_TagHeader);
  int iBorderSize = style()->pixelMetric(customPixelMetrics::PM_TagBorder);

  QFontMetrics fm(font());
  int iWidth = fm.width(text());
  int iHeight = fm.height();

  return QSize(iWidth, iHeight) + QSize(iHeaderSize, 0) + QSize(2 * iBorderSize, 2 * iBorderSize);
}

void TagWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  if (nullptr != parentWidget())
  {
    QStyleOptionTagWidget opt;
    opt.init(this);
    opt.rect = QRect(QPoint(0, 0), m_size);
    opt.color = m_color;

    QPainter painter(this);
    style()->drawControl(customControlElements::CE_TagWidget, &opt, &painter, this);
  }
  else
  {
    QPoint ptCenterOfMass(m_size.width() / 2, m_size.height() / 2);
    double dNullphaseAngleRad = std::atan2(ptCenterOfMass.y() - m_origin.y(),
                              ptCenterOfMass.x() - m_origin.x());

    // create a buffer that has dimensions (2 * diagonal(rect())
    // paint the tag widget into its center
    // rotate the painter based on the simulation
    // paint the buffer to the widget
    QPainter painter(this);
    painter.translate(rect().center());
    painter.rotate((m_dAngleRad - dNullphaseAngleRad) * 180 / c_dPi);
    painter.drawImage(-m_origin, m_image);
  }
}


