#include "groupwidget.h"
#include "ui_groupwidget.h"
#include "taskwidget.h"

#include <QLayout>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QDebug>


GroupWidget::GroupWidget(group_id id, QWidget *parent) :
  QFrame(parent),
  ui(new Ui::GroupWidget),
  m_groupId(id)
{
  ui->setupUi(this);
  connect(ui->pAddTask, &QPushButton::clicked, this, &GroupWidget::onNewTaskClicked);
  connect(ui->pTitle, &EditableLabel::editingFinished, this, &GroupWidget::onTitleEdited);
  connect(ui->pSortTasks, &QPushButton::toggled, this, &GroupWidget::onSortClicked);
  connect(ui->pTaskListWidget, &TaskListWidget::taskInserted, this, &GroupWidget::onTaskInserted);

  setUpContextMenu();
}

GroupWidget::~GroupWidget()
{
  delete ui;
}

void GroupWidget::setUpContextMenu()
{
  setContextMenuPolicy(Qt::ActionsContextMenu);
  for (const auto& pAction : actions())
  {
    removeAction(pAction);
  }

  QAction* pAddTaskAction = new QAction(tr("new task"), this);
  pAddTaskAction->setShortcut(Qt::CTRL + Qt::Key_N);
  pAddTaskAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  addAction(pAddTaskAction);
  connect(pAddTaskAction, SIGNAL(triggered()), this, SLOT(onNewTaskClicked()));
}

void GroupWidget::onTaskInserted(TaskWidget *pTaskWidget, int iPos)
{
  if (nullptr != pTaskWidget)
  {
    emit taskMovedTo(pTaskWidget->id(), m_groupId, iPos);
  }
}

group_id GroupWidget::id() const
{
  return m_groupId;
}

void GroupWidget::setName(const QString& sName)
{
  ui->pTitle->setText(sName);
  setObjectName(QString(sName).remove(" "));

  style()->unpolish(this);
  style()->polish(this);
}

bool GroupWidget::setPropertyValue(const QString& sName, const QString& sValue)
{
  if ("name" == sName)
  {
    setName(sValue);
    return true;
  }

  return false;
}

bool GroupWidget::removeProperty(const QString& /*sName*/)
{
  // no dynamic properties
  return false;
}

std::set<QString> GroupWidget::propertyNames() const
{
  return {"name"};
}

bool GroupWidget::hasPropertyValue(const QString& sName) const
{
  return 0 == sName.compare("name", Qt::CaseSensitive);
}

QString GroupWidget::propertyValue(const QString& sName) const
{
  if ("name" == sName)  { return ui->pTitle->text(); }

  return QString();
}

QImage GroupWidget::backgroundImage() const
{
  return m_backgroundImage;
}

void GroupWidget::setBackgroundImage(const QImage& img)
{
  m_backgroundImage = img;

  ui->pTaskListWidget->setBackgroundImage(img);
}

void GroupWidget::reorderTasks(const std::vector<task_id>& vIds)
{
  ui->pTaskListWidget->reorderTasks(vIds);
}

void GroupWidget::setAutoSortingEnabled(bool bEnabled)
{
  ui->pSortTasks->setChecked(bEnabled);
}

void GroupWidget::requestInsert(TaskWidget *pTaskWidget, int iPos)
{
  ui->pTaskListWidget->requestInsert(pTaskWidget, iPos);
}

bool GroupWidget::insertTask(TaskWidget *pTaskWidget, int iPos)
{
  if (ui->pTaskListWidget->insertTask(pTaskWidget, iPos))
  {
    pTaskWidget->setParentContainerWidget(this);
    return true;
  }
  return false;
}

void GroupWidget::removeTask(TaskWidget *pTaskWidget)
{
  ui->pTaskListWidget->removeTask(pTaskWidget);
  pTaskWidget->setParentContainerWidget(nullptr);
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

bool GroupWidget::onMouseMoved(const QPoint& pt)
{
  return ui->pTaskListWidget->onMouseMoved(ui->pTaskListWidget->mapFrom(this, pt));
}

