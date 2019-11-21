#include "taskcreationdialog.h"
#include "taskwidget.h"

#include <QGridLayout>
#include <QPushButton>
#include <QKeyEvent>

TaskCreationDialog::TaskCreationDialog(QWidget *parent)
  : OverlayWidget(parent)
{
  setTitle(tr("new task"));
}

void TaskCreationDialog::show()
{
  delete m_pTaskWidget;

  QWidget* pWidget(new QWidget());
  QGridLayout* pLayout(new QGridLayout());
  pWidget->setLayout(pLayout);

  m_pTaskWidget = new TaskWidget(-1);
  m_pTaskWidget->setBackgroundImage(QImage(":/new_task_background.png"));
  m_pTaskWidget->setMinimumSize(300, 200);
  m_pTaskWidget->setMaximumSize(300, 200);
  // TODO: lock unnecessary functions
  pLayout->addWidget(m_pTaskWidget, 0, 0, 1, 2);

  QPushButton* pCancel(new QPushButton(tr("Cancel")));
  pCancel->setObjectName("pCancel");
  pCancel->setFocusPolicy(Qt::StrongFocus);
  pCancel->setAutoDefault(true);
  connect(pCancel, SIGNAL(clicked()), this, SLOT(reject()));
  pLayout->addWidget(pCancel, 1, 0);
  QPushButton* pOk(new QPushButton(tr("OK")));
  pOk->setObjectName("pOk");
  pOk->setFocusPolicy(Qt::StrongFocus);
  pOk->setAutoDefault(true);
  connect(pOk, SIGNAL(clicked()), this, SLOT(accept()));
  pLayout->addWidget(pOk, 1, 1);
  pWidget->setObjectName("pInsertTask");
  addWidget(pWidget, Qt::AlignCenter);
  appear();

  QWidget::setTabOrder(pCancel, pOk);
  connect(m_pTaskWidget, SIGNAL(descriptionChanged(task_id, QString)), pOk, SLOT(setFocus()));
  m_pTaskWidget->edit();
}

QString TaskCreationDialog::name() const
{
  return nullptr != m_pTaskWidget ? m_pTaskWidget->name() : QString();
}

QString TaskCreationDialog::description() const
{
  return nullptr != m_pTaskWidget ? m_pTaskWidget->description() : QString();
}

void TaskCreationDialog::keyPressEvent(QKeyEvent* pEvent)
{
  OverlayWidget::keyPressEvent(pEvent);

  switch (pEvent->key())
  {
  case Qt::Key_Escape:
    reject();
  default:
    break;
  }
}

void TaskCreationDialog::accept()
{
  emit accepted();
  disappear();
}

void TaskCreationDialog::reject()
{
  delete m_pTaskWidget;
  m_pTaskWidget = nullptr;

  emit rejected();
  disappear();
}
