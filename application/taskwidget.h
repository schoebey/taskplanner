#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include "id_types.h"
#include "highlightmethod.h"
#include "propertyproviderinterface.h"
#include "itaskcontainerwidget.h"
#include "draggable.h"

#include <QFrame>
#include <QUrl>

#include <map>
#include <set>

namespace Ui {
  class TaskWidget;
}

class TaskListWidget;
class EditableLabel;
class TaskWidgetOverlay;
class QMenu;
class LinkWidget;
class QLabel;
using DraggableTagWidget = Draggable<class TagWidget>;

class TaskWidget :
    public DraggableContainer<DraggableTagWidget>,
    public IPropertyProvider,
    public ITaskContainerWidget
{
  Q_OBJECT

public:
  explicit TaskWidget(task_id id, QWidget *parent = nullptr);
  ~TaskWidget() override;

  task_id id() const;

  QString name() const;
  void setName(const QString& sName);

  QString description() const;
  void setDescription(const QString& sDescription);

  TaskListWidget* previousTaskListWidget() const;
  TaskListWidget* taskListWidget() const;
  void setTaskListWidget(TaskListWidget* pTaskListWidget);

  static TaskWidget* DraggingTaskWidget();
  static void SetTaskWidgetUnderMouse(TaskWidget* pTaskWidget);
  static TaskWidget* TaskWidgetUnderMoue();

  void addProperty(const QString& sName, const QString& sValue);
  bool setPropertyValue(const QString& sName, const QString& sValue) override;
  bool removeProperty(const QString& sName) override;
  std::set<QString> propertyNames() const override;
  bool hasPropertyValue(const QString& sName) const override;
  QString propertyValue(const QString& sName) const override;


  void setHighlight(HighlightingMethod method);
  HighlightingMethod highlight() const;

  void setParentContainerWidget(ITaskContainerWidget *pParentTask);
  ITaskContainerWidget *parentContainerWidget() const;

  void setBackgroundImage(const QImage& image);

  Q_PROPERTY(QBrush overlayBackground READ overlayBackground WRITE setOverlayBackground)
  QBrush overlayBackground() const;
  void setOverlayBackground(const QBrush& b);

  Q_PROPERTY(int collapsedHeight READ collapsedHeight WRITE setCollapsedHeight)
  int collapsedHeight() const;
  void setCollapsedHeight(int iHeight);

  void edit();

  bool onMouseMoved(const QPoint &pt);

  void ensureVisible();

  bool onPropertyValueChanged(const QString& sName, const QString& sValue);

  QSize sizeHint() const;

  QSize minimumSizeHint() const override;
public slots:
  void setExpanded(bool bExpanded);

  void requestInsert(TaskWidget* pTaskWidget, int iPos = -1) override;
  bool insertTask(TaskWidget* pTaskWidget, int iPos = -1, bool bAnimateInsert = true) override;
  void removeTask(TaskWidget* pTaskWidget) override;
  std::vector<TaskWidget*> tasks() const override;
  void ensureVisible(QWidget* pWidget) override;
  void reorderTasks(const std::vector<TaskWidget*>& vpTaskWidgets) override;

  void addTag(const QString& sTag);
  void removeTag(const QString& sTag);

  void addLink(const QUrl& link);
  void removeLink(const QUrl& link);
  void insertLink(const QUrl& link, int iPos);

  void setAutoPriority(double dPriority);

signals:
  void renamed(task_id taskId, const QString& sNewName);
  void descriptionChanged(task_id taskId, const QString& sNewDescription);
  void timeTrackingStarted(task_id id);
  void timeTrackingStopped(task_id id);
  void propertyChanged(task_id id, const QString& sName, const QString& sValue);
  void propertyRemoved(task_id id, const QString& sName);
  void sizeChanged();
  void newSubTaskRequested(task_id taskId);
  void taskMovedTo(task_id taskId, task_id newParentTaskId, int iPos);
  void taskRemoved(task_id parentId, task_id childId);
  void taskDeleted(task_id id);
  void tagAdded(task_id id, const QString& sTag);
  void tagMoved(task_id id, const QString& sTag, task_id sourceTaskId);
  void tagRemoved(task_id id, const QString& sTag);
  void linkAdded(task_id id, QUrl url);
  void linkRemoved(task_id id, QUrl url);
  void linkInserted(task_id id, QUrl url, int iPos);
  void attentionNeeded();
  void priorityUpdateRequested(task_id);
  void addTimeRequested(task_id);
  void removeTimeRequested(task_id);

private slots:
  void onTitleEdited();
  void onDescriptionEdited();
  void on_pStartStop_toggled(bool bOn);
  void setTimeTrackingEnabled(bool bEnabled);
  void onTimeTrackingStopped(task_id id);
  void onPropertyEdited();
  void onAddPropertyTriggered();
  void onRemovePropertyTriggered();
  void updateSize();
  void updateSize2();
  void onDeleteTriggered();
  void onLinkPasted();
  void onAddSubtaskTriggered();
  void onTaskInserted(TaskWidget* pTaskWidget, int iPos = -1);
  void onTaskRemoved(TaskWidget *pTaskWidget);
  void onTagAdded(DraggableTagWidget* pT);
  void onTagMoved(DraggableTagWidget* pT, DraggableContainer<DraggableTagWidget>* pSource);
  void onTagRemoved(DraggableTagWidget* pT);  
private:
  void emphasise();
  void setUpContextMenu();

private:
  Q_DISABLE_COPY(TaskWidget)

  bool eventFilter(QObject* pObj, QEvent* pEvent) override;
  void mousePressEvent(QMouseEvent* pMouseEvent) override;
  void mouseReleaseEvent(QMouseEvent* pEvent) override;
  void mouseMoveEvent(QMouseEvent* pMouseEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;
  void focusInEvent(QFocusEvent* pEvent) override;
  void focusOutEvent(QFocusEvent* pEvent) override;
  void enterEvent(QEvent* pEvent) override;
  void leaveEvent(QEvent* pEvent) override;
  void contextMenuEvent(QContextMenuEvent* pEvent) override;
  void showEvent(QShowEvent* pEvent) override;
  void dragEnterEvent(QDragEnterEvent* pEvent) override;
  void dropEvent(QDropEvent* pEvent) override;
  bool addItem_impl(DraggableTagWidget* pT) override;
  bool removeItem_impl(DraggableTagWidget* pT) override;
  bool insertItem_impl(DraggableTagWidget* pT, QPoint pt) override;
  bool moveItemFrom_impl(DraggableContainer<DraggableTagWidget>* pSource, DraggableTagWidget* pT, QPoint pt) override;

  Ui::TaskWidget *ui;

  task_id m_taskId;

  bool m_bMouseDown = false;
  QPoint m_mouseDownPos;
  double m_dAutoPriority = 0;

  TaskListWidget* m_pTaskListWidget = nullptr;
  TaskListWidget* m_pPreviousTaskListWidget = nullptr;

  TaskWidgetOverlay* m_pOverlay;

  struct SPropertyWidgets
  {
    QFrame* pFrame = nullptr;
    QLabel* pLabel = nullptr;
    EditableLabel* pValue = nullptr;
  };
  std::map<QString, SPropertyWidgets> m_propertyLineEdits;
  std::map<EditableLabel*, QPointer<QTimer>> m_updateTimers;

  ITaskContainerWidget* m_pContainer = nullptr;

  QMenu* m_pContextMenu = nullptr;
  QAction* m_pTrackAction = nullptr;

  std::map<QUrl, LinkWidget*> m_linkWidgets;

  int m_iCollapsedHeight = 35;

  static TaskWidget* m_pDraggingTaskWidget;
  static TaskWidget* m_pTaskWidgetUnderMouse;
};

#endif // TASKWIDGET_H
