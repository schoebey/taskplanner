#include "groupwidget.h"
#include "ui_groupwidget.h"
#include "taskwidget.h"

#include <QLayout>
#include <QResizeEvent>
#include <QPropertyAnimation>

namespace
{
  static const int c_iItemSpacing = 0;

  std::map<QWidget*, QPropertyAnimation*> animations;
  void moveWidget(QWidget* pWidget, QPoint newPos)
  {
    if (nullptr != pWidget && newPos != pWidget->pos())
    {
      QPropertyAnimation* pAnimation = nullptr;
      auto it = animations.find(pWidget);
      if (it != animations.end())
      {
        pAnimation = it->second;
      }
      else
      {
        pAnimation = new QPropertyAnimation(pWidget, "pos");
        animations[pWidget] = pAnimation;
      }

      if (pAnimation->endValue() != newPos ||
          QAbstractAnimation::Running != pAnimation->state())
      {
        pAnimation->stop();
        pAnimation->setDuration(600);
        QPoint currentPos = pWidget->pos();
        pAnimation->setStartValue(currentPos);
        pAnimation->setEndValue(newPos);
        pAnimation->setEasingCurve(QEasingCurve::OutElastic);

        pAnimation->start();
      }
    }
  }
}

GroupWidget* GroupWidget::m_pMouseHoveringOver = nullptr;
GroupWidget::GroupWidget(group_id id, QWidget *parent) :
  QFrame(parent),
  ui(new Ui::GroupWidget),
  m_groupId(id)
{
  ui->setupUi(this);
  connect(ui->pAddTask, SIGNAL(clicked()), this, SLOT(onNewTaskClicked()));
  qApp->installEventFilter(this);
}

GroupWidget::~GroupWidget()
{
  delete ui;
}

group_id GroupWidget::id() const
{
  return m_groupId;
}

void GroupWidget::setCanvas(QWidget* pCanvas)
{
  m_pCanvas = pCanvas;
}

void GroupWidget::InsertTask(TaskWidget* pTaskWidget, int iPos)
{
  if (m_vpTaskWidgets.end() ==
      std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget))
  {
    QPoint origin(mapTo(m_pCanvas, ui->frame->pos()));
    origin.setY(origin.y() + c_iItemSpacing);

    if (-1 == iPos)  { iPos = m_vpTaskWidgets.size(); }


    // find the new widgets position
    for (int iWidget = 0; iWidget < iPos; ++iWidget)
    {
      QWidget* pWidget = m_vpTaskWidgets[iWidget];
      origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
    }


    // insert the new widget
    pTaskWidget->resize(ui->frame->width(), pTaskWidget->minimumSizeHint().height());
    m_vpTaskWidgets.insert(m_vpTaskWidgets.begin() + iPos, pTaskWidget);
    moveWidget(pTaskWidget, origin);
    origin.setY(origin.y() + pTaskWidget->height() + c_iItemSpacing);


    // move the rest of the widgets to the correct place
    for (int iWidget = iPos + 1; iWidget < m_vpTaskWidgets.size(); ++iWidget)
    {
      QWidget* pWidget = m_vpTaskWidgets[iWidget];
      moveWidget(pWidget, origin);
      origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
    }

    pTaskWidget->SetGroupWidget(this);
    pTaskWidget->show();

    ui->frame->setMinimumHeight(ui->frame->mapFrom(m_pCanvas, origin).y());

    emit taskMovedTo(pTaskWidget->id(), m_groupId, iPos);
  }
}

void GroupWidget::RemoveTask(TaskWidget* pTaskWidget)
{
  auto it = std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget);
  if (m_vpTaskWidgets.end() != it)
  {
    QPoint origin(mapTo(m_pCanvas, ui->frame->pos()));
    origin.setY(origin.y() + c_iItemSpacing);

    int iPos = it - m_vpTaskWidgets.begin();
    m_vpTaskWidgets.erase(it);

    // calc the height of the items above the removed one
    for (int iWidget = 0; iWidget < iPos; ++iWidget)
    {
      QWidget* pWidget = m_vpTaskWidgets[iWidget];
      origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
    }

    // move the rest of the widgets to the correct place
    for (int iWidget = iPos; iWidget < m_vpTaskWidgets.size(); ++iWidget)
    {
      QWidget* pWidget = m_vpTaskWidgets[iWidget];
      moveWidget(pWidget, origin);
      origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
    }

    ui->frame->setMinimumHeight(ui->frame->mapFrom(m_pCanvas, origin).y());


    pTaskWidget->SetGroupWidget(nullptr);
    pTaskWidget->show();
  }
}

void GroupWidget::ShowGhost(TaskWidget* pTaskWidget, int iPos)
{
  if (m_vpTaskWidgets.end() ==
      std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget))
  {
    QPoint origin(mapTo(m_pCanvas, ui->frame->pos()));
    origin.setY(origin.y() + c_iItemSpacing);

    if (-1 == iPos)  { iPos = m_vpTaskWidgets.size(); }


    // find the new widgets position
    for (int iWidget = 0; iWidget < std::min<size_t>(iPos, m_vpTaskWidgets.size()); ++iWidget)
    {
      QWidget* pWidget = m_vpTaskWidgets[iWidget];
      moveWidget(pWidget, origin);
      origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
    }

    if (iPos < m_vpTaskWidgets.size())
    {
      // make room for the new widget
      origin.setY(origin.y() + pTaskWidget->height() + c_iItemSpacing);

      // move the rest of the widgets to the correct place
      for (int iWidget = iPos; iWidget < m_vpTaskWidgets.size(); ++iWidget)
      {
        QWidget* pWidget = m_vpTaskWidgets[iWidget];
        moveWidget(pWidget, origin);
        origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
      }
    }

    ui->frame->setMinimumHeight(ui->frame->mapFrom(m_pCanvas, origin).y());
  }
}

GroupWidget* GroupWidget::GroupWidgetUnderMouse()
{
  return m_pMouseHoveringOver;
}

void GroupWidget::repositionChildren()
{
  QPoint origin(mapTo(m_pCanvas, ui->frame->pos()));
  origin.setY(origin.y() + c_iItemSpacing);
  for (auto& pWidget : m_vpTaskWidgets)
  {
    moveWidget(pWidget, origin);
    origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
    pWidget->resize(ui->frame->width(), pWidget->height());
  }

  ui->frame->setMinimumHeight(ui->frame->mapFrom(m_pCanvas, origin).y());
}

void GroupWidget::resizeEvent(QResizeEvent* pEvent)
{
  repositionChildren();
}

void GroupWidget::moveEvent(QMoveEvent* pEvent)
{
  repositionChildren();
}

int GroupWidget::indexFromPoint(QPoint pt)
{
  QPoint globalPt = mapTo(m_pCanvas, pt);
  for (int iIdx = 0; iIdx < m_vpTaskWidgets.size(); ++iIdx)
  {
    QWidget* pWidget = m_vpTaskWidgets[iIdx];
    if (pWidget->mapFrom(m_pCanvas, globalPt).y() < pWidget->rect().center().y())
    {
      return iIdx;
    }
  }

  return m_vpTaskWidgets.size();
}

bool GroupWidget::eventFilter(QObject* pObj, QEvent* pEvent)
{
  if (QEvent::MouseButtonRelease == pEvent->type())
  {
    QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
    if (rect().contains(mapFromGlobal(pMouseEvent->globalPos())))
    {
      if (rect().contains(pMouseEvent->pos()) &&
          nullptr != TaskWidget::DraggingTaskWidget())
      {
        InsertTask(TaskWidget::DraggingTaskWidget());
      }
    }
  }
  else if (QEvent::MouseMove == pEvent->type())
  {
    QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
    QPoint pt = mapFromGlobal(pMouseEvent->globalPos());
    if (rect().contains(pt))
    {
      m_pMouseHoveringOver = this;

      if (nullptr != TaskWidget::DraggingTaskWidget())
      {
        ShowGhost(TaskWidget::DraggingTaskWidget(), indexFromPoint(pt));
      }
    }
    else
    {
      if (m_pMouseHoveringOver == this)
      {
        m_pMouseHoveringOver = nullptr;
      }
      ShowGhost(nullptr, -1);
    }
  }

  return false;
}

void GroupWidget::onNewTaskClicked()
{
  emit newTaskClicked(m_groupId);
}
