#include "groupwidget.h"
#include "ui_groupwidget.h"
#include "taskwidget.h"

#include <QLayout>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QDebug>

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
  connect(ui->pTitle, SIGNAL(editingFinished()), this, SLOT(onTitleEdited()));
  connect(ui->pSortTasks, SIGNAL(clicked(bool)), this, SLOT(onSortClicked(bool)));
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

void GroupWidget::setName(const QString& sName)
{
  ui->pTitle->setText(sName);
}

void GroupWidget::insertTask(TaskWidget* pTaskWidget, int iPos)
{
  if (m_vpTaskWidgets.end() ==
      std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget))
  {
    // insert the new widget
    if (-1 == iPos) iPos = static_cast<int>(m_vpTaskWidgets.size());

    QPoint currentPos = pTaskWidget->mapToGlobal(QPoint(0,0));
    qDebug() << currentPos.x() << currentPos.y();
    pTaskWidget->setParent(ui->scrollAreaWidgetContents);
    pTaskWidget->SetGroupWidget(this);
    pTaskWidget->resize(ui->scrollAreaWidgetContents->width(), pTaskWidget->minimumSizeHint().height());
    pTaskWidget->move(ui->scrollAreaWidgetContents->mapFromGlobal(currentPos));
    pTaskWidget->show();

    connect(pTaskWidget, SIGNAL(sizeChanged()), this, SLOT(repositionChildren()), Qt::QueuedConnection);


    m_vpTaskWidgets.insert(m_vpTaskWidgets.begin() + iPos, pTaskWidget);
    //moveWidget(pTaskWidget, origin);


    UpdatePositions();

    emit taskMovedTo(pTaskWidget->id(), m_groupId, iPos);
  }
}

void GroupWidget::removeTask(TaskWidget* pTaskWidget)
{
  auto it = std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget);
  if (m_vpTaskWidgets.end() != it)
  {
    m_vpTaskWidgets.erase(it);

    UpdatePositions();

    pTaskWidget->SetGroupWidget(nullptr);
    pTaskWidget->show();

    disconnect(pTaskWidget, SIGNAL(sizeChanged()), this, SLOT(repositionChildren()));
  }
}

void GroupWidget::ShowGhost(TaskWidget* pTaskWidget, int iPos)
{
  if (nullptr != pTaskWidget)
  {
    UpdatePositions(pTaskWidget->height(), iPos);
  }
  else
  {
    UpdatePositions();
  }
}

void GroupWidget::UpdatePositions(int iSpace, int iSpacePos)
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
    pWidget->resize(ui->scrollAreaWidgetContents->width(), pWidget->height());
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
    pWidget->resize(ui->scrollAreaWidgetContents->width(), pWidget->height());
    origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
  }

  ui->scrollAreaWidgetContents->setMinimumHeight(origin.y());
}

GroupWidget* GroupWidget::GroupWidgetUnderMouse()
{
  return m_pMouseHoveringOver;
}

void GroupWidget::repositionChildren()
{
  UpdatePositions();
}

void GroupWidget::resizeEvent(QResizeEvent* /*pEvent*/)
{
  repositionChildren();
}

void GroupWidget::moveEvent(QMoveEvent* /*pEvent*/)
{
  repositionChildren();
}

int GroupWidget::indexFromPoint(QPoint pt)
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

TaskWidget* GroupWidget::taskWidgetAt(QPoint pt)
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

QImage GroupWidget::backgroundImage() const
{
  return m_backgroundImage;
}

void GroupWidget::setBackgroundImage(const QImage& img)
{
  m_backgroundImage = img;
}

void GroupWidget::reorderTasks(const std::vector<task_id>& vIds)
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
    UpdatePositions();
  }
}

bool GroupWidget::eventFilter(QObject* /*pObj*/, QEvent* pEvent)
{
  if (QEvent::MouseButtonRelease == pEvent->type())
  {
    QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
    if (ui->scrollAreaWidgetContents->rect().contains(ui->scrollAreaWidgetContents->mapFromGlobal(pMouseEvent->globalPos())))
    {
      if (ui->scrollAreaWidgetContents->rect().contains(ui->scrollAreaWidgetContents->mapFrom(this, pMouseEvent->pos())) &&
          nullptr != TaskWidget::DraggingTaskWidget())
      {
//        insertTask(TaskWidget::DraggingTaskWidget());
      }
    }
  }
  else if (QEvent::MouseMove == pEvent->type())
  {
    QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
    QPoint pt = ui->scrollAreaWidgetContents->mapFromGlobal(pMouseEvent->globalPos());
    if (ui->scrollAreaWidgetContents->rect().contains(pt))
    {
      m_pMouseHoveringOver = this;


      pt = mapFromGlobal(pMouseEvent->globalPos());
      TaskWidget* pTaskWidget = taskWidgetAt(pt);
      TaskWidget::SetTaskWidgetUnderMouse(pTaskWidget);

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

void GroupWidget::onNewTaskClicked()
{
  emit newTaskClicked(m_groupId);
}

void GroupWidget::onTitleEdited()
{
  emit renamed(m_groupId, ui->pTitle->text());
}

void GroupWidget::onSortClicked(bool bChecked)
{
  if (bChecked)
  {
    emit autoSortEnabled(m_groupId);
  }
  else
  {
    emit autoSortDisabled(m_groupId);
  }
}
