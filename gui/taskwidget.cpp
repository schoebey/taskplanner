#include "taskwidget.h"
#include "ui_taskwidget.h"
#include "groupwidget.h"
#include "property.h"
#include "taskwidgetoverlay.h"

#include <QMouseEvent>
#include <QPixmapCache>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QMenu>

#include <QPropertyAnimation>
#include <cassert>
#include <cmath>


TaskWidget* TaskWidget::m_pDraggingTaskWidget = nullptr;
TaskWidget* TaskWidget::m_pTaskWidgetUnderMouse = nullptr;

TaskWidget::TaskWidget(task_id id, QWidget *parent) :
  QFrame(parent),
  ui(new Ui::TaskWidget),
  m_taskId(id),
  m_pOverlay(new TaskWidgetOverlay(this))
{
  ui->setupUi(this);

  connect(this, SIGNAL(sizeChanged()), this, SLOT(updateSize()), Qt::QueuedConnection);
  connect(ui->pTitle, SIGNAL(editingFinished()), this, SLOT(onTitleEdited()));
  connect(ui->pDescription, SIGNAL(editingFinished()), this, SLOT(onDescriptionEdited()));
  connect(ui->pShowDetails, SIGNAL(toggled(bool)), this, SLOT(setExpanded(bool)));

  setUpContextMenu();

  setExpanded(true);
}

TaskWidget::~TaskWidget()
{
  if (m_pTaskWidgetUnderMouse == this)
  {
    m_pTaskWidgetUnderMouse = nullptr;
  }

  if (m_pDraggingTaskWidget == this)
  {
    m_pDraggingTaskWidget = nullptr;
  }

  if (nullptr != m_pGroupWidget)
  {
    m_pGroupWidget->removeTask(this);
  }

  delete ui;
}

void TaskWidget::setUpContextMenu()
{
  if (nullptr == m_pContextMenu)
  {
    m_pContextMenu = new QMenu();
  }

  setContextMenuPolicy(Qt::DefaultContextMenu);
  for (const auto& pAction : m_pContextMenu->actions())
  {
    m_pContextMenu->removeAction(pAction);
    delete pAction;
  }

  QAction* pDeleteAction = new QAction(tr("Delete"), this);
  pDeleteAction->setShortcuts(QList<QKeySequence>() << Qt::Key_Delete << Qt::Key_Backspace);
  pDeleteAction->setShortcutContext(Qt::WidgetShortcut);
  m_pContextMenu->addAction(pDeleteAction);
  connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(onDeleteTriggered()));

  m_pContextMenu->addSeparator();

  QMenu* pPropertiesMenu = m_pContextMenu->addMenu(tr("Add properties"));
  for (const auto& sPropertyName : Properties<Task>::registeredPropertyNames())
  {
    if (m_propertyLineEdits.find(sPropertyName) == m_propertyLineEdits.end() &&
        Properties<Task>::visible(sPropertyName))
    {
      QAction* pAction = new QAction(sPropertyName, this);
      pAction->setProperty("name", sPropertyName);
      connect(pAction, SIGNAL(triggered()), this, SLOT(onAddPropertyTriggered()));
      pPropertiesMenu->addAction(pAction);
    }
  }

  m_pContextMenu->addSeparator();



  QAction* pAction = new QAction(tr("Add subtask"), this);
  connect(pAction, SIGNAL(triggered()), this, SLOT(onAddSubtaskTriggered()));
  m_pContextMenu->addAction(pAction);
}

task_id TaskWidget::id() const
{
  return m_taskId;
}

QString TaskWidget::name() const
{
  return ui->pTitle->text();
}

void TaskWidget::setName(const QString& sName)
{
  ui->pTitle->setText(sName);
}

QString TaskWidget::description() const
{
  return ui->pDescription->text();
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
    setBackgroundImage(m_pGroupWidget->backgroundImage());
    m_pPreviousGroupWidget = m_pGroupWidget;
    m_pDraggingTaskWidget = nullptr; 
  }
}

void TaskWidget::setBackgroundImage(const QImage& image)
{
  m_backgroundImage[1] = m_backgroundImage[0];
  m_backgroundImage[0] = image;
  m_dBackgroundImageBlendFactor = 1;

  QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "backgroundImageBlendFactor");
  static const double c_dDuration = 1000;
  pAnimation->setDuration(c_dDuration);
  pAnimation->setStartValue(1);
  pAnimation->setEndValue(0);
  pAnimation->setEasingCurve(QEasingCurve::Linear);
  pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

double TaskWidget::backgroundImageBlendFactor() const
{
  return m_dBackgroundImageBlendFactor;
}

void TaskWidget::setBackgroundImageBlendFactor(double dFactor)
{
  m_dBackgroundImageBlendFactor = dFactor;
  update();
}

void TaskWidget::edit()
{
  ui->pTitle->edit();
}

TaskWidget*TaskWidget::DraggingTaskWidget()
{
  return m_pDraggingTaskWidget;
}

void TaskWidget::SetTaskWidgetUnderMouse(TaskWidget* pTaskWidget)
{
  if (pTaskWidget != m_pTaskWidgetUnderMouse)
  {
    if (nullptr != m_pTaskWidgetUnderMouse)
    {
      m_pTaskWidgetUnderMouse->setHighlight(m_pTaskWidgetUnderMouse->highlight() &
                                            ~EHighlightMethod::eHover &
                                            ~EHighlightMethod::eInsertPossible);
    }

    m_pTaskWidgetUnderMouse = pTaskWidget;

    if (nullptr != m_pTaskWidgetUnderMouse)
    {
      m_pTaskWidgetUnderMouse->setHighlight(m_pTaskWidgetUnderMouse->highlight() |
                                            EHighlightMethod::eHover |
                                            (nullptr != DraggingTaskWidget() ?
                                             EHighlightMethod::eInsertPossible :
                                             EHighlightMethod::eNoHighlight));
    }
  }
}

TaskWidget*TaskWidget::TaskWidgetUnderMoue()
{
  return m_pTaskWidgetUnderMouse;
}

void TaskWidget::onAddPropertyTriggered()
{
  QObject* pSender = sender();
  if (nullptr != pSender)
  {
    QString sName = pSender->property("name").toString();
    QString sValue;
    addProperty(sName, sValue);
  }
}

void TaskWidget::addProperty(const QString& sName,
                             const QString& sValue)
{
  QLayout* pLayout = ui->pDynamicProperties->layout();
  if (nullptr != pLayout)
  {
    QGridLayout* pGrid = dynamic_cast<QGridLayout*>(pLayout);
    if (nullptr != pGrid)
    {
      QLabel* pLabel = new QLabel(sName);
      EditableLabel* pValue = new EditableLabel(this);
      pValue->setText(sValue);
      pValue->setProperty("name", sName);
      m_propertyLineEdits[sName] = pValue;
      connect(pValue, SIGNAL(editingFinished()), this, SLOT(onPropertyEdited()));
      int iRow = pGrid->rowCount();
      pGrid->addWidget(pLabel, iRow, 0);
      pGrid->addWidget(pValue, iRow, 1);

      emit sizeChanged();
    }
  }

  setUpContextMenu();
}

void TaskWidget::setPropertyValue(const QString& sName, const QString& sValue)
{
  auto it = m_propertyLineEdits.find(sName);
  if (it != m_propertyLineEdits.end())
  {
    it->second->setText(sValue);
  }
}

void TaskWidget::onAddSubtaskTriggered()
{
  emit newSubTaskRequested(id());
}

HighlightingMethod TaskWidget::highlight() const
{
  return m_pOverlay->highlight();
}

void TaskWidget::setParentTask(TaskWidget* pParentTask)
{
  if (pParentTask != m_pParentTask)
  {
    TaskWidget* pFormerParent = m_pParentTask;
    m_pParentTask = pParentTask;
    if (nullptr != pFormerParent)
    {
      pFormerParent->removeTask(this);
    }
    if (nullptr != m_pParentTask)
    {
      m_pParentTask->addTask(this);
    }
  }
}

TaskWidget*TaskWidget::parentTask() const
{
  return m_pParentTask;
}


void TaskWidget::setHighlight(HighlightingMethod method)
{
  m_pOverlay->setHighlight(method);

  if (method.testFlag(EHighlightMethod::eValueRejected))
  {
    QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "pos");
    static const double c_dDuration = 250;
    pAnimation->setDuration(c_dDuration);
    pAnimation->setStartValue(pos() - QPoint(20,0));
    static const double c_dNofShakes = 5;
    for (int i = 0; i < c_dNofShakes; ++i)
    {
      pAnimation->setKeyValueAt(2 * i / c_dNofShakes, pos() - QPoint(20, 0));
      pAnimation->setKeyValueAt((2 * i + 1) / c_dNofShakes, pos() - QPoint(-20, 0));
    }
    pAnimation->setEndValue(pos());
    pAnimation->setEasingCurve(QEasingCurve::Linear);
    pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  }
}

void TaskWidget::updateSize()
{
  resize(width(), sizeHint().height());
}

bool TaskWidget::eventFilter(QObject* /*pObj*/, QEvent* pEvent)
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
      TaskWidget* pTaskWidgetUnderMouse = TaskWidgetUnderMoue();
      if (nullptr != pTaskWidgetUnderMouse)
      {
        // insert dragging task as sub-task...
        pTaskWidgetUnderMouse->addTask(this);
        m_pDraggingTaskWidget = nullptr;
      }
      else if (nullptr != pGroupWidgetUnderMouse)
      {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
        QPoint pt = pGroupWidgetUnderMouse->mapFromGlobal(pMouseEvent->globalPos());
        int iInsertionIndex = pGroupWidgetUnderMouse->indexFromPoint(pt);
        pGroupWidgetUnderMouse->insertTask(this, iInsertionIndex);
      }
      else if (nullptr != m_pPreviousGroupWidget)
      {
        m_pPreviousGroupWidget->insertTask(this);
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
        m_pGroupWidget->removeTask(this);
      }

      if (nullptr != m_pParentTask)
      {
        m_pParentTask->removeTask(this);
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
  resize(width(), sizeHint().height());
  emit descriptionChanged(m_taskId, ui->pDescription->text());
}

void TaskWidget::on_pStartStop_toggled(bool bOn)
{
  if (bOn) {
    emit timeTrackingStarted(m_taskId);
    setHighlight(highlight() | EHighlightMethod::eTimeTrackingActive);
  }
  else {
    emit timeTrackingStopped(m_taskId);
    setHighlight(highlight() & ~EHighlightMethod::eTimeTrackingActive);
  }
}

void TaskWidget::onTimeTrackingStopped(task_id id)
{
  if (m_taskId == id)
  {
    ui->pStartStop->setChecked(false);
  }
}

void TaskWidget::onPropertyEdited()
{
  EditableLabel* pSender = dynamic_cast<EditableLabel*>(sender());
  if (nullptr != pSender)
  {
    QString sPropertyName = pSender->property("name").toString();
    emit propertyChanged(m_taskId, sPropertyName, pSender->text());
  }
}

void TaskWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  bool bRefreshCache = m_cache.isNull() || !qFuzzyIsNull(m_dBackgroundImageBlendFactor);
  if (bRefreshCache)
  {
    m_cache = QPixmap(width(), height());
    m_cache.fill(Qt::transparent);
    QPainter painter(&m_cache);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    static const double c_dBorderOffset = 4.5;
    QRectF rct(rect());
    rct.adjust(c_dBorderOffset, c_dBorderOffset, -c_dBorderOffset, -c_dBorderOffset);

    if (nullptr != m_pParentTask)
    {
      painter.setBrush(QColor(255, 255, 255, 30));
    }
    else
    {
      painter.drawImage(rect(), QImage(":/dropshadow.png"));
      painter.save();
      QPainterPath path;
      path.addRoundedRect(rct, 7, 7);
      painter.setClipPath(path);
      QPointF offset(pos().x()/5, pos().y()/5);
      QBrush b(m_backgroundImage[0]);
      painter.setBrush(b);
      painter.drawRect(rct);


      QBrush f(m_backgroundImage[1]);
      painter.setOpacity(m_dBackgroundImageBlendFactor);
      painter.setBrush(f);
      painter.drawRect(rct);

      painter.restore();
    }

    painter.setPen(QColor(255, 255, 255, 80));
    painter.drawRoundedRect(rct.adjusted(1, 1, -1, -1), 5, 5);
  }
  //else
  {
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_cache);
  }
}

void TaskWidget::resizeEvent(QResizeEvent* pEvent)
{
  QWidget::resizeEvent(pEvent);

  m_pOverlay->move(0, 0);
  m_pOverlay->resize(size());

  m_cache = QPixmap();
}

void TaskWidget::focusInEvent(QFocusEvent* pEvent)
{
  QWidget::focusInEvent(pEvent);
  m_pOverlay->setHighlight(m_pOverlay->highlight() | EHighlightMethod::eFocus);
  SetTaskWidgetUnderMouse(this);
}

void TaskWidget::focusOutEvent(QFocusEvent* pEvent)
{
  QWidget::focusOutEvent(pEvent);
  m_pOverlay->setHighlight(m_pOverlay->highlight() & ~EHighlightMethod::eFocus);
}

void TaskWidget::enterEvent(QEvent* pEvent)
{
  setHighlight(highlight() | EHighlightMethod::eHover);
}

void TaskWidget::leaveEvent(QEvent* pEvent)
{
  setHighlight(highlight() & ~EHighlightMethod::eHover);
}

void TaskWidget::contextMenuEvent(QContextMenuEvent* pEvent)
{

  if (nullptr != m_pContextMenu)
  {
    pEvent->accept();
    m_pContextMenu->exec(pEvent->globalPos());
  }
  else
  {
    pEvent->ignore();
  }
}

void TaskWidget::setExpanded(bool bExpanded)
{
  ui->pProperties->setVisible(bExpanded);
  ui->pShowDetails->setChecked(bExpanded);

  setProperty("expanded", bExpanded);

  emit propertyChanged(m_taskId, "expanded", bExpanded ? "true" : "false");

  ui->pStartStop->style()->unpolish(ui->pStartStop);
  ui->pStartStop->style()->polish(ui->pStartStop);

  QMetaObject::invokeMethod(this, "sizeChanged", Qt::QueuedConnection);
}

void TaskWidget::addTask(TaskWidget* pTaskWidget)
{
  QLayout* pLayout = ui->pSubTasks->layout();
  if (nullptr != pLayout)
  {
    QGridLayout* pGrid = dynamic_cast<QGridLayout*>(pLayout);
    if (nullptr != pGrid)
    {
      pGrid->addWidget(pTaskWidget, pGrid->rowCount(), 0);
      pTaskWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      connect(pTaskWidget, SIGNAL(sizeChanged()), this, SIGNAL(sizeChanged()), Qt::QueuedConnection);
      pTaskWidget->setParentTask(this);

      emit taskAdded(id(), pTaskWidget->id());

      emit sizeChanged();
    }
  }
}

void TaskWidget::removeTask(TaskWidget* pTaskWidget)
{
  disconnect(pTaskWidget, SIGNAL(sizeChanged()), this, SIGNAL(sizeChanged()));
  pTaskWidget->setParentTask(nullptr);

  emit taskRemoved(id(), pTaskWidget->id());

  ui->pSubTasks->layout()->removeWidget(pTaskWidget);

  QMetaObject::invokeMethod(this, "sizeChanged", Qt::QueuedConnection);
}

void TaskWidget::onDeleteTriggered()
{
  emit taskDeleted(id());
}
