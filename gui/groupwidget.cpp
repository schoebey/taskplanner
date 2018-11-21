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

void GroupWidget::InsertTask(TaskWidget* pTaskWidget, int iPos)
{
  if (m_vpTaskWidgets.end() ==
      std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget))
  {
    // insert the new widget
    if (-1 == iPos) iPos = m_vpTaskWidgets.size();

    QPoint currentPos = pTaskWidget->mapToGlobal(QPoint(0,0));
    qDebug() << currentPos.x() << currentPos.y();
    pTaskWidget->setParent(ui->scrollAreaWidgetContents);
    pTaskWidget->SetGroupWidget(this);
    pTaskWidget->resize(ui->scrollAreaWidgetContents->width(), pTaskWidget->minimumSizeHint().height());
    pTaskWidget->move(ui->scrollAreaWidgetContents->mapFromGlobal(currentPos));
    pTaskWidget->show();


    m_vpTaskWidgets.insert(m_vpTaskWidgets.begin() + iPos, pTaskWidget);
    //moveWidget(pTaskWidget, origin);


    UpdatePositions();

    emit taskMovedTo(pTaskWidget->id(), m_groupId, iPos);
  }
}

void GroupWidget::RemoveTask(TaskWidget* pTaskWidget)
{
  auto it = std::find(m_vpTaskWidgets.begin(), m_vpTaskWidgets.end(), pTaskWidget);
  if (m_vpTaskWidgets.end() != it)
  {
    m_vpTaskWidgets.erase(it);

    UpdatePositions();

    pTaskWidget->SetGroupWidget(nullptr);
    pTaskWidget->show();
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

  if (-1 == iSpacePos)  { iSpacePos = m_vpTaskWidgets.size(); }


  // find the new widgets position
  for (int iWidget = 0; iWidget < std::min<size_t>(iSpacePos, m_vpTaskWidgets.size()); ++iWidget)
  {
    QWidget* pWidget = m_vpTaskWidgets[iWidget];
    moveWidget(pWidget, origin);
    pWidget->resize(ui->scrollAreaWidgetContents->width(), pWidget->height());
    origin.setY(origin.y() + pWidget->height() + c_iItemSpacing);
  }

  if (-1 < iSpace && iSpacePos < m_vpTaskWidgets.size())
  {
    // make room for the new widget
    origin.setY(origin.y() + iSpace + c_iItemSpacing);
  }


  for (int iWidget = iSpacePos; iWidget < m_vpTaskWidgets.size(); ++iWidget)
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
  for (int iIdx = 0; iIdx < m_vpTaskWidgets.size(); ++iIdx)
  {
    QWidget* pWidget = m_vpTaskWidgets[iIdx];
    if (pWidget->mapFrom(this, pt).y() < pWidget->rect().center().y())
    {
      return iIdx;
    }
  }

  return m_vpTaskWidgets.size();
}

QImage GroupWidget::backgroundImage() const
{
  return m_backgroundImage;
}

void GroupWidget::setBackgroundImage(const QImage& img)
{
  m_backgroundImage = img;
}

bool GroupWidget::eventFilter(QObject* pObj, QEvent* pEvent)
{
  if (QEvent::MouseButtonRelease == pEvent->type())
  {
    QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
    if (ui->scrollAreaWidgetContents->rect().contains(ui->scrollAreaWidgetContents->mapFromGlobal(pMouseEvent->globalPos())))
    {
      if (ui->scrollAreaWidgetContents->rect().contains(ui->scrollAreaWidgetContents->mapFrom(this, pMouseEvent->pos())) &&
          nullptr != TaskWidget::DraggingTaskWidget())
      {
        InsertTask(TaskWidget::DraggingTaskWidget());
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

      if (nullptr != TaskWidget::DraggingTaskWidget())
      {
        pt = mapFromGlobal(pMouseEvent->globalPos());
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

void GroupWidget::onTitleEdited()
{
  emit renamed(m_groupId, ui->pTitle->text());
}
