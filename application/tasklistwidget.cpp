#include "tasklistwidget.h"
#include "taskwidget.h"

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
  : QFrame(pParent)
{
  qApp->installEventFilter(this);
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
    if (-1 == iPos || iPos >= m_vpTaskWidgets.size())
    {
      iPos = static_cast<int>(m_vpTaskWidgets.size());
    }

    QPoint currentPos = pTaskWidget->mapToGlobal(QPoint(0,0));
    qDebug() << currentPos.x() << currentPos.y();
    pTaskWidget->setParent(this);
    pTaskWidget->setTaskListWidget(this);
    pTaskWidget->resize(width(), pTaskWidget->sizeHint().height());
    pTaskWidget->move(mapFromGlobal(currentPos));
    pTaskWidget->show();

    connect(pTaskWidget, SIGNAL(sizeChanged()), this, SLOT(repositionChildren()), Qt::QueuedConnection);


    m_vpTaskWidgets.insert(m_vpTaskWidgets.begin() + iPos, pTaskWidget);

    pTaskWidget->show();

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

    disconnect(pTaskWidget, SIGNAL(sizeChanged()), this, SLOT(repositionChildren()));
  }
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

  if (vpTaskWidgets != m_vpTaskWidgets)
  {
    m_vpTaskWidgets = vpTaskWidgets;
    QMetaObject::invokeMethod(this, "UpdatePositions", Qt::QueuedConnection);
  }
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

void TaskListWidget::resizeEvent(QResizeEvent */*pEvent*/)
{
  updatePositions();
}

void TaskListWidget::moveEvent(QMoveEvent */*pEvent*/)
{
  updatePositions();
}

void TaskListWidget::updatePositions(int iSpace, int iSpacePos)
{
  QPoint origin(0,0);
  origin.setY(origin.y() + c_iItemSpacing);

  size_t spacePos = 0;
  if (-1 == iSpacePos)
  {
    spacePos = m_vpTaskWidgets.size();
  }
  else
  {
    spacePos = static_cast<size_t>(iSpacePos);
  }


  // find the new widgets position
  for (size_t iWidget = 0; iWidget < std::min<size_t>(spacePos, m_vpTaskWidgets.size()); ++iWidget)
  {
    QWidget* pWidget = m_vpTaskWidgets[iWidget];
    moveWidget(pWidget, origin);
    pWidget->resize(width(), pWidget->sizeHint().height());
    origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
  }

  if (-1 < iSpace && spacePos < m_vpTaskWidgets.size())
  {
    // make room for the new widget
    origin.setY(origin.y() + iSpace + c_iItemSpacing);
  }


  for (size_t iWidget = spacePos; iWidget < m_vpTaskWidgets.size(); ++iWidget)
  {
    QWidget* pWidget = m_vpTaskWidgets[iWidget];
    moveWidget(pWidget, origin);
    pWidget->resize(width(), pWidget->sizeHint().height());
    origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
  }

  setMinimumHeight(origin.y());
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

bool TaskListWidget::eventFilter(QObject* /*pObj*/, QEvent* pEvent)
{
  if (QEvent::MouseMove == pEvent->type())
  {
    QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
    QPoint pt = mapFromGlobal(pMouseEvent->globalPos());
    if (rect().contains(pt))
    {
      pt = mapFromGlobal(pMouseEvent->globalPos());
      TaskWidget* pTaskWidget = taskWidgetAt(pt);
      TaskWidget::SetTaskWidgetUnderMouse(pTaskWidget);

      if (nullptr != TaskWidget::DraggingTaskWidget())
      {
        if (this != m_pMouseHoveringOver)
        {
          TaskWidget::DraggingTaskWidget()->setBackgroundImage(backgroundImage());
        }

        // if dragging the widget into the drop zone
        // of the task (for sub tasks), the ghost should
        // not change position.
        int iPos = indexFromPoint(pt);
        if (-1 != iPos)
        {
          ShowGhost(TaskWidget::DraggingTaskWidget(), iPos);
        }
      }

      m_pMouseHoveringOver = this;
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
  }

  return false;
}
