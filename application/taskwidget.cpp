#include "taskwidget.h"
#include "ui_taskwidget.h"
#include "property.h"
#include "taskwidgetoverlay.h"
#include "flowlayout.h"
#include "linkwidget.h"
#include "mousehandlingframe.h"
#include "task.h"
#include "tasklistwidget.h"
#include "decoratedlabel.h"
#include "groupwidget.h"
#include "propertyeditorfactory.h"

#include <QMouseEvent>
#include <QPixmapCache>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QMenu>
#include <QMimeData>
#include <QClipboard>
#include <QPointer>

#include <QPropertyAnimation>
#include <cassert>
#include <cmath>


namespace widgetAnimation {
  void deleteWidgetAnimation(QWidget*);
}

namespace {
  void forceUpdate(QWidget* pWidget)
  {
    pWidget->style()->unpolish(pWidget);
    pWidget->style()->polish(pWidget);
    pWidget->update();
  }
}


TaskWidget* TaskWidget::m_pDraggingTaskWidget = nullptr;
TaskWidget* TaskWidget::m_pTaskWidgetUnderMouse = nullptr;

TaskWidget::TaskWidget(task_id id, QWidget *parent) :
  QFrame(parent),
  ui(new Ui::TaskWidget),
  m_taskId(id)
{
  ui->setupUi(this);
  ui->pTaskListWidget->setAutoResize(true);
  m_pOverlay = new TaskWidgetOverlay(ui->pFrame);
  m_pOverlay->stackUnder(ui->pProperties);

  auto pShadow = new FloatingWidget(this);
  pShadow->setObjectName("pShadow");
  pShadow->stackUnder(ui->pBackdrop);

  FlowLayout* pFlowLayout = new FlowLayout(ui->pLinks, 0, 0, 0);
  ui->pLinks->setLayout(pFlowLayout);

  connect(this, SIGNAL(sizeChanged()), this, SLOT(updateSize()), Qt::QueuedConnection);
  connect(ui->pTitle, SIGNAL(editingFinished()), this, SLOT(onTitleEdited()));
  connect(ui->pDescription, SIGNAL(editingFinished()), this, SLOT(onDescriptionEdited()));
  connect(ui->pDescription, SIGNAL(sizeChanged()), this, SLOT(updateSize()));
  connect(ui->pShowDetails, SIGNAL(toggled(bool)), this, SLOT(setExpanded(bool)));
  connect(ui->pTaskListWidget, &TaskListWidget::taskInserted, this, &TaskWidget::onTaskInserted);
  connect(ui->pTaskListWidget, &TaskListWidget::taskRemoved, this, &TaskWidget::onTaskRemoved);
  connect(ui->pTaskListWidget, &TaskListWidget::sizeChanged, this, &TaskWidget::updateSize, Qt::QueuedConnection);
  connect(this, &TaskWidget::attentionNeeded, this, &TaskWidget::emphasise);

  setUpContextMenu();

  setExpanded(true);
  setAcceptDrops(true);
}

TaskWidget::~TaskWidget()
{
  widgetAnimation::deleteWidgetAnimation(this);

  if (m_pTaskWidgetUnderMouse == this)
  {
    m_pTaskWidgetUnderMouse = nullptr;
  }

  if (m_pDraggingTaskWidget == this)
  {
    m_pDraggingTaskWidget = nullptr;
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


  // TODO: shortcuts don't work in general. why? focus problem? can't be,
  // because the context is set to application...
  // problem with hierarchy? maybe qt is stupid if widgets aren't present
  // in a traditional parent-child hierarchy...
  QAction* pPasteLinkAction = new QAction(tr("Paste link from clipboard"), this);
  pPasteLinkAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
  pPasteLinkAction->setShortcutContext(Qt::WidgetShortcut);
  addAction(pPasteLinkAction);
  m_pContextMenu->addAction(pPasteLinkAction);
  connect(pPasteLinkAction, SIGNAL(triggered()), this, SLOT(onLinkPasted()));

  QAction* pDeleteAction = new QAction(tr("Delete"), this);
  pDeleteAction->setShortcuts(QList<QKeySequence>() << Qt::Key_Delete << Qt::Key_Backspace);
  pDeleteAction->setShortcutContext(Qt::WidgetShortcut);
  m_pContextMenu->addAction(pDeleteAction);
  addAction(pDeleteAction);
  connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(onDeleteTriggered()));

  m_pContextMenu->addSeparator();

  QMenu* pPropertiesMenu = m_pContextMenu->addMenu(tr("Add properties"));
  for (const auto& sPropertyName : Properties<Task>::registeredPropertyNames())
  {
    if (m_propertyEditors.find(sPropertyName) == m_propertyEditors.end() &&
        Properties<Task>::visible(sPropertyName))
    {
      QAction* pAction = new QAction(sPropertyName, this);
      pAction->setProperty("name", sPropertyName);
      connect(pAction, SIGNAL(triggered()), this, SLOT(onAddPropertyTriggered()));
      pPropertiesMenu->addAction(pAction);
    }
  }

  if (!m_propertyEditors.empty())
  {
    QMenu* pRemovePropertiesMenu = m_pContextMenu->addMenu(tr("Remove properties"));
    for (const auto& sPropertyName : Properties<Task>::registeredPropertyNames())
    {
      if (m_propertyEditors.find(sPropertyName) != m_propertyEditors.end() &&
          Properties<Task>::visible(sPropertyName))
      {
        QAction* pAction = new QAction(sPropertyName, this);
        pAction->setProperty("name", sPropertyName);
        connect(pAction, SIGNAL(triggered()), this, SLOT(onRemovePropertyTriggered()));
        pRemovePropertiesMenu->addAction(pAction);
      }
    }
  }

  m_pContextMenu->addSeparator();



  QAction* pAction = new QAction(tr("Add subtask"), this);
  connect(pAction, SIGNAL(triggered()), this, SLOT(onAddSubtaskTriggered()));
  m_pContextMenu->addAction(pAction);


  m_pContextMenu->addSeparator();
  m_pTrackAction = new QAction(ui->pStartStop->isChecked() ?
                                                tr("Stop tracking") :
                                                tr("Start tracking"), this);
  m_pTrackAction->setShortcut(Qt::Key_T);
  m_pTrackAction->setShortcutContext(Qt::WidgetShortcut);
  addAction(m_pTrackAction);
  m_pContextMenu->addAction(m_pTrackAction);
  connect(m_pTrackAction, &QAction::triggered, this, [&]()
  {
    setTimeTrackingEnabled(!ui->pStartStop->isChecked());
  }
  );

  QAction* pAddTimeAction = new QAction(tr("Add time"), this);
  connect(pAddTimeAction, &QAction::triggered, this, [&](){ emit addTimeRequested(id()); });
  m_pContextMenu->addAction(pAddTimeAction);

  QAction* pRemoveTimeAction = new QAction(tr("Remove time"), this);
  connect(pRemoveTimeAction, &QAction::triggered, this, [&](){ emit removeTimeRequested(id()); });
  m_pContextMenu->addAction(pRemoveTimeAction);
}

task_id TaskWidget::id() const
{
  return m_taskId;
}

QString TaskWidget::name() const
{
  return ui->pTitle->editText();
}

void TaskWidget::setName(const QString& sName)
{
  ui->pTitle->setEditText(sName);
}

QString TaskWidget::description() const
{
  return ui->pDescription->editText();
}

void TaskWidget::setDescription(const QString& sDescription)
{
  ui->pDescription->setEditText(sDescription);
  updateSize();
}

TaskListWidget* TaskWidget::previousTaskListWidget() const
{
  return m_pPreviousTaskListWidget;
}

TaskListWidget* TaskWidget::taskListWidget() const
{
  return m_pTaskListWidget;
}

void TaskWidget::setTaskListWidget(TaskListWidget* pTaskListWidget)
{
  if (m_pTaskListWidget != pTaskListWidget)
  {
    if (nullptr != m_pTaskListWidget)
    {
      m_pTaskListWidget->removeTask(this);
    }

    m_pTaskListWidget = pTaskListWidget;

    if (nullptr != m_pTaskListWidget)
    {
      setBackgroundImage(m_pTaskListWidget->backgroundImage());
      m_pPreviousTaskListWidget = m_pTaskListWidget;
      m_pDraggingTaskWidget = nullptr;
    }
  }
}

void TaskWidget::setBackgroundImage(const QImage& image)
{
  ui->pBackdrop->setBackgroundImage(image);
}

void TaskWidget::dragEnterEvent(QDragEnterEvent* pEvent)
{
  pEvent->setAccepted(pEvent->mimeData()->hasUrls());
}

void TaskWidget::dropEvent(QDropEvent* pEvent)
{
  for (const auto& url : pEvent->mimeData()->urls())
  {
    emit linkAdded(m_taskId, url);
  }
}

QBrush TaskWidget::overlayBackground() const
{
  return m_pOverlay->background();
}

void TaskWidget::setOverlayBackground(const QBrush& b)
{
  m_pOverlay->setBackground(b);
}

void TaskWidget::edit()
{
  connect(ui->pTitle, &EditableLabel::editingFinished, ui->pDescription, &EditableLabel::edit);
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

void TaskWidget::onRemovePropertyTriggered()
{
  QObject* pSender = sender();
  if (nullptr != pSender)
  {
    QString sName = pSender->property("name").toString();
    removeProperty(sName);

    emit propertyRemoved(m_taskId, sName);
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
      QFrame* pFrame = new MouseHandlingFrame();
      m_propertyEditors[sName].pFrame = pFrame;
      pFrame->setObjectName("pPropertyFrame");
      QHBoxLayout* pHboxLayout = new QHBoxLayout();
      pHboxLayout->setSpacing(0);
      pHboxLayout->setMargin(0);
      pFrame->setLayout(pHboxLayout);
      QLabel* pLabel = new DecoratedLabel(sName);
      pLabel->setFocusPolicy(Qt::NoFocus);
      pLabel->setObjectName(QString("%1_label").arg(sName));
      m_propertyEditors[sName].pLabel = pLabel;


      QWidget* pEditor = PropertyEditorFactory::createAndConnect<TaskWidget>(sName, this);
      if (nullptr != pEditor)
      {
        connect(pEditor, SIGNAL(attentionNeeded()), this, SIGNAL(attentionNeeded()));
        connect(pFrame, SIGNAL(mouseDoubleClicked(QPoint)), pEditor, SLOT(edit()));
        pEditor->setObjectName(QString("%1_value").arg(sName));
        pEditor->setProperty("name", sName);
      }

      m_propertyEditors[sName].pValue = pEditor;


      //connect(pValue, SIGNAL(editingFinished()), this, SLOT(onPropertyEdited()));
      if (nullptr != pEditor)
      {
        int iRow = pGrid->rowCount();
        pHboxLayout->addWidget(pLabel);
        pHboxLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
        pHboxLayout->addWidget(pEditor);
        pGrid->addWidget(pFrame, iRow, 0);
        pFrame->setVisible(true);

        if (!sValue.isEmpty())
        {
          setPropertyValue(sName, sValue);
          onPropertyValueChanged(sName, sValue);
        }
      } 
      else {
		setPropertyValue(sName, sValue);
        onPropertyValueChanged(sName, sValue);
      }

      updateSize();
    }
  }

  setUpContextMenu();
}

std::set<QString> TaskWidget::propertyNames() const
{
  std::set<QString> names = {"name", "description"};
  for (const auto& el : m_propertyEditors)
  {
    names.insert(el.first);
  }
  return names;
}

bool TaskWidget::hasPropertyValue(const QString& sName) const
{
  auto it = m_propertyEditors.find(sName);
  return it != m_propertyEditors.end();
}

QString TaskWidget::propertyValue(const QString& sName) const
{
  if ("name" == sName)  { return name(); }
  else if ("description" == sName)  { return description(); }
  else
  {
    auto it = m_propertyEditors.find(sName);
    if (it != m_propertyEditors.end())
    {
      return it->second.sValue;
    }
  }

  return QString();
}

bool TaskWidget::removeProperty(const QString& sName)
{
  auto it = m_propertyEditors.find(sName);
  if (it != m_propertyEditors.end())
  {
    it->second.pLabel->deleteLater();
    it->second.pValue->deleteLater();
    it->second.pFrame->deleteLater();
    m_propertyEditors.erase(it);

    updateSize();
    return true;
  }

  setUpContextMenu();

  return false;
}

bool TaskWidget::setPropertyValue(const QString& sName, const QString& sValue)
{
  emit propertyChanged(m_taskId, sName, sValue);

  return true;
}

bool TaskWidget::onPropertyValueChanged(const QString& sName, const QString& sValue)
{
  auto it = m_propertyLineEdits.find(sName);
  if (it != m_propertyLineEdits.end())
  {
    if (sValue.isEmpty())
    {
      it->second.pLabel->deleteLater();
      it->second.pValue->deleteLater();
      it->second.pFrame->deleteLater();
      m_propertyEditors.erase(it);
      updateSize();

      setUpContextMenu();
    }
    else
    {
      emit propertyValueChanged(sName, sValue);
      it->second.sValue = sValue;
    }

    return true;
  }
  else if ("description" == sName)
  {
    setDescription(sValue);
    return true;
  }
  else if ("name" == sName)
  {
    setName(sValue);
    return true;
  }
  else if ("expanded" == sName)
  {
    bool bOk(false);
    bool bExpanded = conversion::fromString<bool>(sValue, bOk);
    if (bOk)  { setExpanded(bExpanded); }
  }
  else if ("links" == sName)
  {
    bool bOk(false);
    auto links = conversion::fromString<std::vector<QUrl>>(sValue, bOk);
    if (bOk)
    {
      // all links that are not yet present have to be added
      for (const auto& link : links)
      {
        auto it = m_linkWidgets.find(link);
        if (it == m_linkWidgets.end())
        {
          addLink(link);
        }
      }

      // all links that are present but not represented in sValue have to be removed
      std::set<QUrl> toRemove;
      for (auto it = m_linkWidgets.begin(); it != m_linkWidgets.end(); ++it)
      {
        auto itProp = std::find(links.begin(), links.end(), it->first);
        if (itProp == links.end())
        {
          toRemove.insert(it->first);
        }
      }

      for (const auto& url : toRemove)  { removeLink(url); }
    }
  }
  else
  {
    if (Properties<Task>::visible(sName))
    {
      if (!sValue.isEmpty())
      {
        addProperty(sName, sValue);
      }
      return true;
    }
  }

  return false;
}

void TaskWidget::addLink(const QUrl& link)
{
  QLayout* pLayout = ui->pLinks->layout();
  if (nullptr != pLayout)
  {
    if (m_linkWidgets.end() == m_linkWidgets.find(link))
    {
      LinkWidget* pLinkWidget = new LinkWidget(link);
      pLayout->addWidget(pLinkWidget);

      connect(pLinkWidget, SIGNAL(deleteTriggered(QUrl)), this, SLOT(removeLink(QUrl)));

      m_linkWidgets[link] = pLinkWidget;
    }
  }
}

void TaskWidget::removeLink(const QUrl& link)
{
  auto it = m_linkWidgets.find(link);
  if (it != m_linkWidgets.end())
  {
    it->second->deleteLater();
    m_linkWidgets.erase(it);
  }

  emit linkRemoved(m_taskId, link);
}

void TaskWidget::insertLink(const QUrl& link, int iPos)
{
  assert(false && "todo");
  emit linkInserted(m_taskId, link, iPos);
}

void TaskWidget::setAutoPriority(double dPriority)
{
  if (!qFuzzyCompare(m_dAutoPriority, dPriority))
  {
    m_dAutoPriority = dPriority;

    // TODO: tell the overlay widget that the prio has changed
    // let it change its color/property based on the new prio
    // have priority buckets for high/medium/low?
    int iPrioCategory = static_cast<int>(dPriority);
    if (iPrioCategory != m_pOverlay->property("autoPriority").toInt())
    {
      m_pOverlay->style()->unpolish(m_pOverlay);
      m_pOverlay->setProperty("autoPriority", iPrioCategory);
      m_pOverlay->style()->polish(m_pOverlay);
      m_pOverlay->update();


      setProperty("autoPriority", iPrioCategory);
      forceUpdate(this);
      forceUpdate(ui->pShowDetails);
      forceUpdate(ui->pStartStop);

      for (auto* pLabel : findChildren<QLabel*>())
      {
        forceUpdate(pLabel);
      }
    }
  }
}

void TaskWidget::onAddSubtaskTriggered()
{
  emit newSubTaskRequested(id());
}

void TaskWidget::onTaskInserted(TaskWidget *pTaskWidget, int iPos)
{
  if (nullptr != pTaskWidget)
  {
    emit taskMovedTo(pTaskWidget->id(), m_taskId, iPos);

    updateSize();
  }
}

void TaskWidget::onTaskRemoved(TaskWidget* /*pTaskWidget*/)
{
  updateSize();
}

HighlightingMethod TaskWidget::highlight() const
{
  return m_pOverlay->highlight();
}

void TaskWidget::setParentContainerWidget(ITaskContainerWidget* pContainer)
{
  if (pContainer != m_pContainer)
  {
    auto pFormerContainer = m_pContainer;
    m_pContainer = pContainer;
    if (nullptr != pFormerContainer)
    {
      pFormerContainer->removeTask(this);
    }
    if (nullptr != m_pContainer)
    {
      m_pContainer->insertTask(this);
    }
  }
}

ITaskContainerWidget* TaskWidget::parentContainerWidget() const
{
  return m_pContainer;
}


void TaskWidget::setHighlight(HighlightingMethod method)
{
  m_pOverlay->setHighlight(method);

  setProperty("highlight", static_cast<int>(method));

  for (QWidget* pChildWidget : std::vector<QWidget*>{this, ui->pTitle, ui->pDescription})
  {
    pChildWidget->style()->unpolish(pChildWidget);
    pChildWidget->style()->polish(pChildWidget);
  }

  if (method.testFlag(EHighlightMethod::eValueRejected))
  {
    QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "pos");
    static const int c_iDuration = 250;
    pAnimation->setDuration(c_iDuration);
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
  int iWidth = ui->pProperties->width();
  ui->pDescription->suggestWidth(iWidth);

  QMetaObject::invokeMethod(this, "updateSize2", Qt::QueuedConnection);
}

void TaskWidget::updateSize2()
{
  layout()->invalidate();
  layout()->update();


  int iSuggestedHeight = ui->pBackdrop->sizeHint().height();
  resize(width(), iSuggestedHeight);
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
      TaskListWidget* pTaskListWidgetUnderMouse = TaskListWidget::TaskListWidgetUnderMouse();
      TaskWidget* pTaskWidgetUnderMouse = TaskWidgetUnderMoue();
      if (nullptr != pTaskListWidgetUnderMouse)
      {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
        QPoint pt = pTaskListWidgetUnderMouse->mapFromGlobal(pMouseEvent->globalPos());
        int iInsertionIndex = pTaskListWidgetUnderMouse->indexFromPoint(pt);
        pTaskListWidgetUnderMouse->requestInsert(this, iInsertionIndex);
      }
      else if (nullptr != pTaskWidgetUnderMouse)
      {
        // append dragging task as sub-task...
        pTaskWidgetUnderMouse->requestInsert(this);
      }
      else if (nullptr != m_pPreviousTaskListWidget)
      {
        m_pPreviousTaskListWidget->insertTask(this);
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

      if (nullptr != m_pContainer)
      {
        m_pContainer->removeTask(this);
      }

      qApp->installEventFilter(this);
      setParent(window());
      setFocus();
      move(pMouseEvent->globalPos() - m_mouseDownPos);
      show();
      raise();
    }
  }
}


void TaskWidget::onTitleEdited()
{
  emit renamed(m_taskId, ui->pTitle->editText());
}

void TaskWidget::onDescriptionEdited()
{
  emit descriptionChanged(m_taskId, ui->pDescription->editText());
  updateSize();
}

void TaskWidget::on_pStartStop_toggled(bool bOn)
{
  setTimeTrackingEnabled(bOn);
}

void TaskWidget::setTimeTrackingEnabled(bool bEnabled)
{
  ui->pStartStop->setChecked(bEnabled);

  if (bEnabled) {
    m_pTrackAction->setText(tr("Stop tracking"));
    emit timeTrackingStarted(m_taskId);
    setHighlight(highlight() | EHighlightMethod::eTimeTrackingActive);
  }
  else {
    m_pTrackAction->setText(tr("Start tracking"));
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
    setPropertyValue(sPropertyName, pSender->editText());
  }
}

void TaskWidget::onPropertyValueChanged(const QString& sPropertyName,
                                        const QString& sValue)
{
  emit propertyChanged(m_taskId, sPropertyName, sValue);
}

void TaskWidget::resizeEvent(QResizeEvent* pEvent)
{
  QWidget::resizeEvent(pEvent);

//  m_cache = QPixmap();

  emit sizeChanged();
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

void TaskWidget::enterEvent(QEvent* /*pEvent*/)
{
  setHighlight(highlight() | EHighlightMethod::eHover);
}

void TaskWidget::leaveEvent(QEvent* /*pEvent*/)
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

  if (property("expanded").toBool() ^ bExpanded)
  {
    setProperty("expanded", bExpanded);

    setPropertyValue("expanded", bExpanded ? "true" : "false");

    if (bExpanded)
    {
      resize(m_expandedSize);
    }
    else
    {
      m_expandedSize = size();
      resize(width(), 1);
    }


    ui->pStartStop->style()->unpolish(ui->pStartStop);
    ui->pStartStop->style()->polish(ui->pStartStop);

    updateSize();
  }
}

void TaskWidget::requestInsert(TaskWidget *pTaskWidget, int iPos)
{
  ui->pTaskListWidget->requestInsert(pTaskWidget, iPos);
}

bool TaskWidget::insertTask(TaskWidget *pTaskWidget, int iPos, bool bAnimateInsert)
{
  if (ui->pTaskListWidget->insertTask(pTaskWidget, iPos, bAnimateInsert))
  {
    pTaskWidget->setParentContainerWidget(this);
    return true;
  }
  return false;
}

void TaskWidget::removeTask(TaskWidget *pTaskWidget)
{
  pTaskWidget->setParentContainerWidget(nullptr);
  ui->pTaskListWidget->removeTask(pTaskWidget);
  updateSize();
}

std::vector<TaskWidget*> TaskWidget::tasks() const
{
  return ui->pTaskListWidget->tasks();
}

void TaskWidget::ensureVisible(QWidget* pWidget)
{
  if (isAncestorOf(pWidget))
  {
    setExpanded(true);
    m_pContainer->ensureVisible(pWidget);
  }
}

void TaskWidget::onDeleteTriggered()
{
  emit taskDeleted(id());
}

void TaskWidget::onLinkPasted()
{
  QClipboard* pClipboard = QApplication::clipboard();

  if (pClipboard->mimeData()->hasUrls())
  {
    for (const auto& url : pClipboard->mimeData()->urls())
    {
      emit linkAdded(m_taskId, url);
    }
  }
  else if (pClipboard->mimeData()->hasText())
  {
    for (const auto& sLink: pClipboard->mimeData()->text().split("\n"))
    {
      QUrl url(sLink);

      // checking a QUrl for validity is pointless since everything seems to be valid...
      static const QRegExp c_rx(R"(^(?:http(s)?:\/\/)?[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+$)");
      if (url.isLocalFile() ||
          0 == c_rx.indexIn(url.toString()))
      {
        emit linkAdded(m_taskId, sLink);
      }
    }
  }
}

void TaskWidget::showEvent(QShowEvent* /*pEvent*/)
{
  updateSize();
}

bool TaskWidget::onMouseMoved(const QPoint& pt)
{
  bool bRv = ui->pTaskListWidget->onMouseMoved(ui->pTaskListWidget->mapFrom(this, pt));

  static const int c_iBorderSize = 20;
  if (!bRv && rect().adjusted(0, c_iBorderSize, 0, -c_iBorderSize).contains(pt))
  {
    TaskWidget::SetTaskWidgetUnderMouse(this);
    return true;
  }

  return bRv;
}

void TaskWidget::ensureVisible()
{
  m_pContainer->ensureVisible(this);
}

void TaskWidget::emphasise()
{
  QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "pos");
  static const int c_iDuration = 700;
  pAnimation->setDuration(c_iDuration);
  pAnimation->setStartValue(pos() - QPoint(0,50));
//  static const double c_dNofShakes = 5;
//  for (int i = 0; i < c_dNofShakes; ++i)
//  {
//    pAnimation->setKeyValueAt(2 * i / c_dNofShakes, pos() - QPoint(20, 0));
//    pAnimation->setKeyValueAt((2 * i + 1) / c_dNofShakes, pos() - QPoint(-20, 0));
//  }
  pAnimation->setEndValue(pos());
  pAnimation->setEasingCurve(QEasingCurve::OutBounce);
  pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}


void TaskWidget::reorderTasks(const std::vector<TaskWidget*>& vpTaskWidgets)
{
  ui->pTaskListWidget->reorderTasks(vpTaskWidgets);
}
