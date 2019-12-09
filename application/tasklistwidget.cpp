#include "tasklistwidget.h"
#include "taskwidget.h"

#include <QStyle>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
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

namespace widgetAnimation
{
  void deleteWidgetAnimation(QWidget* pWidget)
  {
    auto it = animations.find(pWidget);
    if (it != animations.end())
    {
      animations.erase(it);
    }
  }
}

TaskListWidget* TaskListWidget::m_pMouseHoveringOver = nullptr;
TaskListWidget::TaskListWidget(QWidget *pParent)
  : QFrame(pParent),
    m_pGhost(new QFrame(this))
{
  qApp->installEventFilter(this);
  m_pGhost->setObjectName("pGhost");
}

TaskListWidget *TaskListWidget::TaskListWidgetUnderMouse()
{
  return m_pMouseHoveringOver;
}


void TaskListWidget::requestInsert(TaskWidget* pTaskWidget, int iPos)
{
  if (m_vpTaskWidgets.end() ==
      std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget))
  {
    // insert the new widget
    if (-1 == iPos) iPos = static_cast<int>(m_vpTaskWidgets.size());

    emit taskInserted(pTaskWidget, iPos);
  }
}

bool TaskListWidget::insertTask(TaskWidget* pTaskWidget, int iPos)
{
  if (m_vpTaskWidgets.end() ==
      std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget))
  {
    // insert the new widget
    if (-1 == iPos || iPos >= static_cast<int>(m_vpTaskWidgets.size()))
    {
      iPos = static_cast<int>(m_vpTaskWidgets.size());
    }

    QPoint currentPos = pTaskWidget->mapToGlobal(QPoint(0,0));
    auto bHasFocus = pTaskWidget->hasFocus();
    pTaskWidget->setParent(this);
    pTaskWidget->setTaskListWidget(this);
    pTaskWidget->resize(width(), pTaskWidget->sizeHint().height());
    pTaskWidget->move(mapFromGlobal(currentPos));
    if (bHasFocus)  { pTaskWidget->setFocus(); }
    pTaskWidget->show();

    connect(pTaskWidget, &TaskWidget::sizeChanged, this, &TaskListWidget::updateTaskPositions, Qt::QueuedConnection);


    m_vpTaskWidgets.insert(m_vpTaskWidgets.begin() + iPos, pTaskWidget);

    pTaskWidget->show();
    pTaskWidget->style()->unpolish(pTaskWidget);
    pTaskWidget->style()->polish(pTaskWidget);

    QMetaObject::invokeMethod(this, "updatePositions", Qt::QueuedConnection);

    return true;
  }

  return false;
}

void TaskListWidget::removeTask(TaskWidget* pTaskWidget)
{
  auto it = std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget);
  if (m_vpTaskWidgets.end() != it)
  {
    m_vpTaskWidgets.erase(it);

    QMetaObject::invokeMethod(this, "updatePositions", Qt::QueuedConnection);

    pTaskWidget->setTaskListWidget(nullptr);
    pTaskWidget->show();

    disconnect(pTaskWidget, &TaskWidget::sizeChanged, this, &TaskListWidget::updateTaskPositions);
  }
}

std::vector<TaskWidget*> TaskListWidget::tasks() const
{
  return m_vpTaskWidgets;
}

void TaskListWidget::ensureVisible(QWidget* /*pWidget*/)
{
  // task list widget always shows all its child task widgets - nothing to do here.
}

void TaskListWidget::setBackgroundImage(const QImage &img)
{
  m_backgroundImage = img;
  for (auto pTaskWidget : m_vpTaskWidgets)
  {
    pTaskWidget->setBackgroundImage(img);
  }
}

QImage TaskListWidget::backgroundImage()
{
  return m_backgroundImage;
}

void TaskListWidget::reorderTasks(const std::vector<TaskWidget*>& vpTaskWidgets)
{
  if (vpTaskWidgets != m_vpTaskWidgets)
  {
    m_vpTaskWidgets = vpTaskWidgets;
    QMetaObject::invokeMethod(this, "updatePositions", Qt::QueuedConnection);
  }
}

void TaskListWidget::reorderTasks(const std::vector<task_id>& vIds)
{
  std::vector<TaskWidget*> vpTaskWidgets;
  for (const auto& id : vIds)
  {
    auto it = std::find_if(m_vpTaskWidgets.begin(),
                           m_vpTaskWidgets.end(),
                           [id](const TaskWidget* pTaskWidget)
    {
      return nullptr != pTaskWidget &&
          pTaskWidget->id() == id;
    });
    if (it != m_vpTaskWidgets.end())
    {
      vpTaskWidgets.push_back(*it);
    }
  }

  reorderTasks(vpTaskWidgets);
}

void TaskListWidget::ShowGhost(TaskWidget* pTaskWidget, int iPos)
{
  if (nullptr != pTaskWidget)
  {
    updatePositions(pTaskWidget->height(), iPos);
  }
  else
  {
    updatePositions();
  }
}

TaskWidget *TaskListWidget::taskWidgetAt(QPoint pt)
{
  for (size_t iIdx = 0; iIdx < m_vpTaskWidgets.size(); ++iIdx)
  {
    TaskWidget* pWidget = m_vpTaskWidgets[iIdx];
    QRect widgetRect(pWidget->rect());
    if (widgetRect.contains(pWidget->mapFrom(this, pt)))
    {
      return pWidget;
    }
  }

  return nullptr;
}

void TaskListWidget::resizeEvent(QResizeEvent* pEvent)
{
  QFrame::resizeEvent(pEvent);

  updatePositions();

  emit sizeChanged();
}

void TaskListWidget::moveEvent(QMoveEvent* /*pEvent*/)
{
  updatePositions();
}

void TaskListWidget::updateTaskPositions()
{
  updatePositions(-1, 0);
}

QSize TaskListWidget::minimumSizeHint() const
{
  return m_minimumSize;
}

QSize TaskListWidget::sizeHint() const
{
  return m_minimumSize;
}

void TaskListWidget::updatePositions(int iSpace, int iGhostPos)
{

  QPoint origin(0,0);
  origin.setY(origin.y() + c_iItemSpacing);

  size_t ghostPos = 0;
  if (-1 == iGhostPos)
  {
    ghostPos = m_vpTaskWidgets.size();
  }
  else
  {
    ghostPos = static_cast<size_t>(iGhostPos);
  }


  // find the new widgets position
  for (size_t iWidget = 0; iWidget < std::min<size_t>(ghostPos, m_vpTaskWidgets.size()); ++iWidget)
  {
    QWidget* pWidget = m_vpTaskWidgets[iWidget];
    moveWidget(pWidget, origin);
    pWidget->resize(width(), pWidget->height());
    origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
  }

  if (-1 < iSpace && ghostPos < m_vpTaskWidgets.size())
  {
    m_pGhost->move(0, origin.y());

    // make room for the new widget
    origin.setY(origin.y() + iSpace + c_iItemSpacing);
  }

  m_pGhost->resize(width(), iSpace);


  for (size_t iWidget = ghostPos; iWidget < m_vpTaskWidgets.size(); ++iWidget)
  {
    QWidget* pWidget = m_vpTaskWidgets[iWidget];
    moveWidget(pWidget, origin);
    pWidget->resize(width(), pWidget->height());
    origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
  }



  // only resize the list if it is a nested list (one within a task widget)
  // group widget lists have to have maximum height at all times...
  if (m_bAutoResize)
  {
    setMinimumHeight(origin.y());
    setMaximumHeight(origin.y());
    m_minimumSize = QSize(width(), origin.y());
    updateGeometry();
  }
  else
  {
    setMinimumHeight(origin.y());
  }
}

void TaskListWidget::setSize(int iWidth, int iHeight)
{
  resize(iWidth, iHeight);
}

int TaskListWidget::indexFromPoint(QPoint pt)
{
  static const int c_iBorderSize = 20;
  for (size_t iIdx = 0; iIdx < m_vpTaskWidgets.size(); ++iIdx)
  {
    QWidget* pWidget = m_vpTaskWidgets[iIdx];
    QRect widgetRect(pWidget->rect());
    int iRelativeY = pWidget->mapFrom(this, pt).y();
    if (iRelativeY < widgetRect.top() + c_iBorderSize)
    {
      return static_cast<int>(iIdx);
    }
    else if (iRelativeY < widgetRect.bottom() - c_iBorderSize)
    {
      // "neutral" zone of the widget
      return -1;
    }
  }

  return static_cast<int>(m_vpTaskWidgets.size());
}

bool TaskListWidget::onMouseMoved(const QPoint& pt)
{
  if (rect().contains(pt))
  {
    if (nullptr != TaskWidget::DraggingTaskWidget())
    {
      if (this != m_pMouseHoveringOver)
      {
        TaskWidget::DraggingTaskWidget()->setBackgroundImage(backgroundImage());
      }
    }

    TaskWidget* pTaskWidget = taskWidgetAt(pt);
    if (nullptr != pTaskWidget)
    {
      m_pMouseHoveringOver = nullptr;
      if (pTaskWidget->onMouseMoved(pTaskWidget->mapFrom(this, pt)))
      {
        ShowGhost(nullptr, -1);
        return true;
      }

      if (nullptr != TaskWidget::DraggingTaskWidget())
      {
        // if dragging the widget into the drop zone
        // of the task (for sub tasks), the ghost should
        // not change position.
        int iPos = indexFromPoint(pt);
        if (-1 != iPos)
        {
          ShowGhost(TaskWidget::DraggingTaskWidget(), iPos);
        }
      }
    }
    else
    {
      TaskWidget::SetTaskWidgetUnderMouse(nullptr);
    }

    m_pMouseHoveringOver = this;

    return true;
  }
  else
  {
    if (m_pMouseHoveringOver == this)
    {
      m_pMouseHoveringOver = nullptr;
      TaskWidget::SetTaskWidgetUnderMouse(nullptr);
    }
    ShowGhost(nullptr, -1);
  }

  return false;
}

void TaskListWidget::setAutoResize(bool bAutoResize)
{
  m_bAutoResize = bAutoResize;
}
