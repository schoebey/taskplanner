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
    assert(0 == newPos.x());
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

void GroupWidget::InsertTask(TaskWidget* pTaskWidget, int iPos)
{
  if (m_vpTaskWidgets.end() ==
      std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget))
  {
    int iPosY = c_iItemSpacing;

    if (-1 == iPos)  { iPos = m_vpTaskWidgets.size(); }


    // find the new widgets position
    for (int iWidget = 0; iWidget < iPos; ++iWidget)
    {
      QWidget* pWidget = m_vpTaskWidgets[iWidget];
      iPosY += pWidget->height() + c_iItemSpacing;
    }


    // insert the new widget
    QPoint currentPos = pTaskWidget->mapToGlobal(QPoint(0,0));
    pTaskWidget->setParent(ui->frame);
    currentPos = ui->frame->mapFromGlobal(currentPos);
    pTaskWidget->move(currentPos);
    m_vpTaskWidgets.insert(m_vpTaskWidgets.begin() + iPos, pTaskWidget);
    moveWidget(pTaskWidget, QPoint(0, iPosY));
    iPosY += pTaskWidget->height() + c_iItemSpacing;


    // move the rest of the widgets to the correct place
    for (int iWidget = iPos + 1; iWidget < m_vpTaskWidgets.size(); ++iWidget)
    {
      QWidget* pWidget = m_vpTaskWidgets[iWidget];
      moveWidget(pWidget, QPoint(0, iPosY));
      iPosY += pWidget->height() + c_iItemSpacing;
    }

    pTaskWidget->SetGroupWidget(this);
    pTaskWidget->show();

    ui->frame->setMinimumHeight(iPosY);

    emit taskMovedTo(pTaskWidget->id(), m_groupId, iPos);
  }
}

void GroupWidget::RemoveTask(TaskWidget* pTaskWidget)
{
  auto it = std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget);
  if (m_vpTaskWidgets.end() != it)
  {
    int iPosY = c_iItemSpacing;

    int iPos = it - m_vpTaskWidgets.begin();
    m_vpTaskWidgets.erase(it);

    // calc the height of the items above the removed one
    for (int iWidget = 0; iWidget < iPos; ++iWidget)
    {
      QWidget* pWidget = m_vpTaskWidgets[iWidget];
      iPosY += pWidget->height() + c_iItemSpacing;
    }

    // move the rest of the widgets to the correct place
    for (int iWidget = iPos; iWidget < m_vpTaskWidgets.size(); ++iWidget)
    {
      QWidget* pWidget = m_vpTaskWidgets[iWidget];
      moveWidget(pWidget, QPoint(0, iPosY));
      iPosY += pWidget->height() + c_iItemSpacing;
    }

    ui->frame->setMinimumHeight(iPosY);


    pTaskWidget->SetGroupWidget(nullptr);
    pTaskWidget->setParent(window());
    pTaskWidget->show();
  }
}

void GroupWidget::ShowGhost(TaskWidget* pTaskWidget, int iPos)
{
  if (m_vpTaskWidgets.end() ==
      std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget))
  {
    int iPosY = c_iItemSpacing;

    if (-1 == iPos)  { iPos = m_vpTaskWidgets.size(); }


    // find the new widgets position
    for (int iWidget = 0; iWidget < iPos; ++iWidget)
    {
      QWidget* pWidget = m_vpTaskWidgets[iWidget];
      moveWidget(pWidget, QPoint(0, iPosY));
      iPosY += pWidget->height() + c_iItemSpacing;
    }

    if (iPos < m_vpTaskWidgets.size())
    {
      // make room for the new widget
      iPosY += pTaskWidget->height() + c_iItemSpacing;

      // move the rest of the widgets to the correct place
      for (int iWidget = iPos; iWidget < m_vpTaskWidgets.size(); ++iWidget)
      {
        QWidget* pWidget = m_vpTaskWidgets[iWidget];
        moveWidget(pWidget, QPoint(0, iPosY));
        iPosY += pWidget->height() + c_iItemSpacing;
      }
    }

    ui->frame->setMinimumHeight(iPosY);
  }
}

GroupWidget* GroupWidget::GroupWidgetUnderMouse()
{
  return m_pMouseHoveringOver;
}

void GroupWidget::resizeEvent(QResizeEvent* pEvent)
{
  for (auto& pWidget : m_vpTaskWidgets)
  {
    pWidget->resize(ui->frame->width(), pWidget->height());
  }
}

int GroupWidget::indexFromPoint(QPoint pt)
{
  int iY = 0;
  for (int iIdx = 0; iIdx < m_vpTaskWidgets.size(); ++iIdx)
  {
    QWidget* pWidget = m_vpTaskWidgets[iIdx];
    if (pWidget->mapFrom(this, pt).y() < pWidget->rect().center().y())
    {
      return iIdx;
    }
  }
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
