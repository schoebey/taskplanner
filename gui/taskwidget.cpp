#include "taskwidget.h"
#include "ui_taskwidget.h"
#include "groupwidget.h"

#include <QMouseEvent>
#include <QPixmapCache>
#include <QPainter>

#include <cassert>
#include <cmath>

namespace
{

  void drawShadowedText(QPainter* pPainter, const QPoint& pt, const QString& sText, QColor shadowColor)
  {
    if (nullptr == pPainter)  { return; }

    QFont f(pPainter->font());
    f.setPointSize(20);
    QFontMetrics m(f);

    QString sKey = QString("%1_%2x%3_%4").arg(sText).arg(m.width(sText)).arg(m.height()).arg(shadowColor.name());

    QPixmap* pPixmap = QPixmapCache::find(sKey);
    if (nullptr == pPixmap)
    {
      QImage img(m.width(sText), m.height() + 2, QImage::Format_ARGB32);
      img.fill(Qt::transparent);

      QPainterPath textPath;
      textPath.addText(0, img.height()-2, f, sText);

      QPainterPath shadowPath;
      shadowPath.addText(0, img.height()-1, f, sText);

      QPainter p(&img);
      p.setPen(Qt::NoPen);
      p.setBrush(shadowColor);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::HighQualityAntialiasing, true);
      p.setRenderHint(QPainter::SmoothPixmapTransform, true);
      p.drawPath(shadowPath);
      p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
      p.setBrush(QColor(0,0,0,255));
      p.drawPath(textPath);
      p.setCompositionMode(QPainter::CompositionMode_SourceOver);
      p.setBrush(QColor(255,255,255,80));
      p.drawPath(textPath);
      QPixmapCache::insert(sKey, QPixmap::fromImage(img));
      pPixmap = QPixmapCache::find(sKey);
    }


    if (nullptr != pPixmap)
    {
      pPainter->drawPixmap(pt.x(), pt.y(), *pPixmap);
    }
  }
}


TaskWidget* TaskWidget::m_pDraggingTaskWidget = nullptr;

TaskWidget::TaskWidget(task_id id, QWidget *parent) :
  QFrame(parent),
  ui(new Ui::TaskWidget),
  m_taskId(id)
{
  ui->setupUi(this);

  connect(ui->pTitle, SIGNAL(editingFinished()), this, SLOT(onTitleEdited()));
  connect(ui->pDescription, SIGNAL(editingFinished()), this, SLOT(onDescriptionEdited()));
}

TaskWidget::~TaskWidget()
{
  delete ui;
}

task_id TaskWidget::id() const
{
  return m_taskId;
}

void TaskWidget::setName(const QString& sName)
{
  ui->pTitle->setText(sName);
}

void TaskWidget::setDescription(const QString& sDescription)
{
  ui->pDescription->setText(sDescription);
}

void TaskWidget::SetGroupWidget(GroupWidget* pGroupWidget)
{
  m_pGroupWidget = pGroupWidget;

  if (nullptr != m_pGroupWidget)
  {
    m_backgroundImage = m_pGroupWidget->backgroundImage();
    m_pPreviousGroupWidget = m_pGroupWidget;
    m_pDraggingTaskWidget = nullptr;
  }
}

TaskWidget*TaskWidget::DraggingTaskWidget()
{
  return m_pDraggingTaskWidget;
}

bool TaskWidget::eventFilter(QObject* pObj, QEvent* pEvent)
{
  if (this == m_pDraggingTaskWidget)
  {
    if (QEvent::MouseMove == pEvent->type())
    {
      QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
      move(parentWidget()->mapFromGlobal(pMouseEvent->globalPos() - m_mouseDownPos));
    }
    else if (QEvent::MouseButtonRelease == pEvent->type())
    {
      m_bMouseDown = false;
      GroupWidget* pGroupWidgetUnderMouse = GroupWidget::GroupWidgetUnderMouse();
      if (nullptr != pGroupWidgetUnderMouse)
      {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
        QPoint pt = pGroupWidgetUnderMouse->mapFromGlobal(pMouseEvent->globalPos());
        int iInsertionIndex = pGroupWidgetUnderMouse->indexFromPoint(pt);
        pGroupWidgetUnderMouse->InsertTask(this, iInsertionIndex);
      }
      else if (nullptr != m_pPreviousGroupWidget)
      {
        m_pPreviousGroupWidget->InsertTask(this);
      }
      else
      {
        assert(false);
      }

      qApp->removeEventFilter(this);
    }
  }

  return false;
}

void TaskWidget::mousePressEvent(QMouseEvent* pMouseEvent)
{
  QFrame::mousePressEvent(pMouseEvent);

  m_bMouseDown = pMouseEvent->buttons() & Qt::LeftButton;
  if (m_bMouseDown)
  {
    m_mouseDownPos = pMouseEvent->pos();
  }
}

void TaskWidget::mouseMoveEvent(QMouseEvent* pMouseEvent)
{
  QFrame::mouseMoveEvent(pMouseEvent);

  if (m_bMouseDown && nullptr == m_pDraggingTaskWidget)
  {
    QPoint ptDist = pMouseEvent->pos() - m_mouseDownPos;
    if (20 < std::sqrt(ptDist.x() * ptDist.x() + ptDist.y() * ptDist.y()))
    {
      m_pDraggingTaskWidget = this;

      if (nullptr != m_pGroupWidget)
      {
        m_pGroupWidget->RemoveTask(this);
      }

      qApp->installEventFilter(this);
      setParent(window());
      move(pMouseEvent->globalPos() - m_mouseDownPos);
      show();
      raise();
    }
  }
}


void TaskWidget::onTitleEdited()
{
  emit renamed(m_taskId, ui->pTitle->text());
}

void TaskWidget::onDescriptionEdited()
{
  int iMinHeight = minimumSizeHint().height();
  resize(width(), iMinHeight);
  emit descriptionChanged(m_taskId, ui->pDescription->text());
}

void TaskWidget::on_pStartStop_toggled(bool bOn)
{
  ui->pStartStop->setText(bOn ? "stop" : "start");

  if (bOn) {
    emit timeTrackingStarted(m_taskId);
  }
  else {
    emit timeTrackingStopped(m_taskId);
  }
}

void TaskWidget::paintEvent(QPaintEvent* pEvent)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  painter.drawImage(rect(), QImage(":/dropshadow.png"));

  static const int c_iBorderOffset = 5;
  QRectF rct(rect().adjusted(c_iBorderOffset, c_iBorderOffset, -c_iBorderOffset, -c_iBorderOffset));

  QPainterPath path;
  path.addRoundedRect(rct, 5, 5);
  painter.setClipPath(path);
  QPointF offset(pos().x()/5, pos().y()/5);
  painter.drawImage(rct, m_backgroundImage,
                    rct.adjusted(offset.x(), offset.y(), offset.x(), offset.y()));



//  drawShadowedText(&painter, QPoint(100,0), "hello world", QColor(0,0,0,100));

//  drawShadowedText(&painter, QPoint(100,20), "hello world", QColor(255,0,0,100));
}
