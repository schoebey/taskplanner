#include "taskwidget.h"
#include "ui_taskwidget.h"
#include "groupwidget.h"
#include "property.h"
#include "taskwidgetoverlay.h"
#include "flowlayout.h"
#include "linkwidget.h"
#include "mousehandlingframe.h"

#include <QMouseEvent>
#include <QPixmapCache>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QMenu>
#include <QMimeData>
#include <QClipboard>

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

  FlowLayout* pFlowLayout = new FlowLayout(ui->pLinks, 0, 0, 0);
  ui->pLinks->setLayout(pFlowLayout);

  connect(this, SIGNAL(sizeChanged()), this, SLOT(updateSize()), Qt::QueuedConnection);
  connect(ui->pTitle, SIGNAL(editingFinished()), this, SLOT(onTitleEdited()));
  connect(ui->pDescription, SIGNAL(editingFinished()), this, SLOT(onDescriptionEdited()));
  connect(ui->pDescription, SIGNAL(sizeChanged()), this, SLOT(updateSize()));
  connect(ui->pShowDetails, SIGNAL(toggled(bool)), this, SLOT(setExpanded(bool)));

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
    if (m_propertyLineEdits.find(sPropertyName) == m_propertyLineEdits.end() &&
        Properties<Task>::visible(sPropertyName))
    {
      QAction* pAction = new QAction(sPropertyName, this);
      pAction->setProperty("name", sPropertyName);
      connect(pAction, SIGNAL(triggered()), this, SLOT(onAddPropertyTriggered()));
      pPropertiesMenu->addAction(pAction);
    }
  }

  if (!m_propertyLineEdits.empty())
  {
    QMenu* pRemovePropertiesMenu = m_pContextMenu->addMenu(tr("Remove properties"));
    for (const auto& sPropertyName : Properties<Task>::registeredPropertyNames())
    {
      if (m_propertyLineEdits.find(sPropertyName) != m_propertyLineEdits.end() &&
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
  emit sizeChanged();
}

GroupWidget*TaskWidget::previousGroupWidget() const
{
  return m_pPreviousGroupWidget;
}

GroupWidget*TaskWidget::groupWidget() const
{
  return m_pGroupWidget;
}

void TaskWidget::setGroupWidget(GroupWidget* pGroupWidget)
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
  static const int c_iDuration = 1000;
  pAnimation->setDuration(c_iDuration);
  pAnimation->setStartValue(1);
  pAnimation->setEndValue(0);
  pAnimation->setEasingCurve(QEasingCurve::Linear);
  pAnimation->start(QAbstractAnimation::DeleteWhenStopped);

  for (auto pSubTask : m_subTasks)
  {
    pSubTask->setBackgroundImage(image);
  }
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

bool TaskWidget::dropShadow() const
{
  return m_bDropShadow;
}

void TaskWidget::setDropShadow(bool bOn)
{
  m_bDropShadow = bOn;
  m_cache = QPixmap();
  update();
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
      m_propertyLineEdits[sName].pFrame = pFrame;
      pFrame->setObjectName("pPropertyFrame");
      QHBoxLayout* pHboxLayout = new QHBoxLayout();
      pHboxLayout->setSpacing(0);
      pHboxLayout->setMargin(0);
      pFrame->setLayout(pHboxLayout);
      QLabel* pLabel = new QLabel(sName);
      pLabel->setFocusPolicy(Qt::NoFocus);
      pLabel->setObjectName(QString("%1_label").arg(sName));
      m_propertyLineEdits[sName].pLabel = pLabel;

      EditableLabel* pValue = new EditableLabel(this);
      connect(pFrame, SIGNAL(mouseDoubleClicked(QPoint)), pValue, SLOT(edit()));
      pValue->setObjectName(QString("%1_value").arg(sName));
      pValue->setFocusPolicy(Qt::NoFocus);
      pValue->setProperty("name", sName);
      pValue->setAlignment(Qt::AlignCenter);
      m_propertyLineEdits[sName].pValue = pValue;

      connect(pValue, SIGNAL(editingFinished()), this, SLOT(onPropertyEdited()));
      int iRow = pGrid->rowCount();
      pHboxLayout->addWidget(pLabel);
      pHboxLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
      pHboxLayout->addWidget(pValue);
      pGrid->addWidget(pFrame, iRow, 0);

      setPropertyValue(sName, sValue);

      emit sizeChanged();
    }
  }

  setUpContextMenu();
}

std::set<QString> TaskWidget::propertyNames() const
{
  std::set<QString> names = {"name", "description"};
  for (const auto& el : m_propertyLineEdits)
  {
    names.insert(el.first);
  }
  return names;
}

bool TaskWidget::hasPropertyValue(const QString& sName) const
{
  auto it = m_propertyLineEdits.find(sName);
  return it != m_propertyLineEdits.end();
}

QString TaskWidget::propertyValue(const QString& sName) const
{
  if ("name" == sName)  { return name(); }
  else if ("description" == sName)  { return description(); }
  else
  {
    auto it = m_propertyLineEdits.find(sName);
    if (it != m_propertyLineEdits.end())
    {
      return it->second.pValue->text();
    }
  }

  return QString();
}

bool TaskWidget::removeProperty(const QString& sName)
{
  auto it = m_propertyLineEdits.find(sName);
  if (it != m_propertyLineEdits.end())
  {
    delete it->second.pLabel;
    delete it->second.pValue;
    delete it->second.pFrame;
    m_propertyLineEdits.erase(it);

    emit sizeChanged();
    return true;
  }

  setUpContextMenu();

  return false;
}

bool TaskWidget::setPropertyValue(const QString& sName, const QString& sValue)
{
  auto it = m_propertyLineEdits.find(sName);
  if (it != m_propertyLineEdits.end())
  {
    if (sValue.isEmpty())
    {
      delete it->second.pLabel;
      delete it->second.pValue;
      delete it->second.pFrame;
      m_propertyLineEdits.erase(it);
      emit sizeChanged();

      setUpContextMenu();
    }
    else
    {
      it->second.pValue->setText(sValue);
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
    m_pOverlay->setProperty("autoPriority", static_cast<int>(dPriority));
    m_pOverlay->style()->unpolish(m_pOverlay);
    m_pOverlay->style()->polish(m_pOverlay);
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
  int iSuggestedHeight = sizeHint().height();
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
        pGroupWidgetUnderMouse->requestInsert(this, iInsertionIndex);
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
  emit descriptionChanged(m_taskId, ui->pDescription->text());
  emit sizeChanged();
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
      if (m_bDropShadow)
      {
        painter.drawImage(rect(), QImage(":/dropshadow.png"));
      }
      painter.save();
      QPainterPath path;
      path.addRoundedRect(rct, 7, 7);
      painter.setClipPath(path);
      QPointF offset(pos().x()/5, pos().y()/5);
      QBrush b(m_backgroundImage[0]);
      painter.setPen(Qt::NoPen);
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

    emit propertyChanged(m_taskId, "expanded", bExpanded ? "true" : "false");

    ui->pStartStop->style()->unpolish(ui->pStartStop);
    ui->pStartStop->style()->polish(ui->pStartStop);

    emit sizeChanged();
  }
}

void TaskWidget::addTask(TaskWidget* pTaskWidget)
{
  if (m_subTasks.end() == m_subTasks.find(pTaskWidget))
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

        m_subTasks.insert(pTaskWidget);

        pTaskWidget->show();
      }
    }
  }
}

void TaskWidget::removeTask(TaskWidget* pTaskWidget)
{
  disconnect(pTaskWidget, SIGNAL(sizeChanged()), this, SIGNAL(sizeChanged()));
  pTaskWidget->setParentTask(nullptr);

  emit taskRemoved(id(), pTaskWidget->id());

  ui->pSubTasks->layout()->removeWidget(pTaskWidget);

  emit sizeChanged();

  m_subTasks.erase(pTaskWidget);
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
  emit sizeChanged();
}
