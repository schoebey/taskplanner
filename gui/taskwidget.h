#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include "id_types.h"
#include "highlightmethod.h"

#include <QFrame>
#include <QUrl>

#include <map>
#include <set>

namespace Ui {
  class TaskWidget;
}

class GroupWidget;
class EditableLabel;
class TaskWidgetOverlay;
class QMenu;
class LinkWidget;
class TaskWidget : public QFrame
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

  void setGroupWidget(GroupWidget* pGroupWidget);

  static TaskWidget* DraggingTaskWidget();
  static void SetTaskWidgetUnderMouse(TaskWidget* pTaskWidget);
  static TaskWidget* TaskWidgetUnderMoue();

  void addProperty(const QString& sName, const QString& sValue);
  void setPropertyValue(const QString& sName, const QString& sValue);

  void addLink(const QUrl& link);
  void removeLink(const QUrl& link);
  void insertLink(const QUrl& link, int iPos);

  void setHighlight(HighlightingMethod method);
  HighlightingMethod highlight() const;

  void setParentTask(TaskWidget* pParentTask);
  TaskWidget* parentTask() const;

  void setBackgroundImage(const QImage& image);

  Q_PROPERTY(double backgroundImageBlendFactor READ backgroundImageBlendFactor WRITE setBackgroundImageBlendFactor)
  double backgroundImageBlendFactor() const;
  void setBackgroundImageBlendFactor(double dFactor);

  Q_PROPERTY(bool dropShadow READ dropShadow WRITE setDropShadow)
  bool dropShadow() const;
  void setDropShadow(bool bOn);

  void edit();
public slots:
  void setExpanded(bool bExpanded);
  void addTask(TaskWidget* pTaskWidget);
  void removeTask(TaskWidget* pTaskWidget);

signals:
  void renamed(task_id taskId, const QString& sNewName);
  void descriptionChanged(task_id taskId, const QString& sNewDescription);
  void timeTrackingStarted(task_id id);
  void timeTrackingStopped(task_id id);
  void propertyChanged(task_id id, const QString& sName, const QString& sValue);
  void sizeChanged();
  void newSubTaskRequested(task_id taskId);
  void taskAdded(task_id parentId, task_id childId);
  void taskRemoved(task_id parentId, task_id childId);
  void taskDeleted(task_id id);
  void linkAdded(task_id id, QUrl url);
  void linkRemoved(task_id id, QUrl url);
  void linkInserted(task_id id, QUrl url, int iPos);

private slots:
  void onTitleEdited();
  void onDescriptionEdited();
  void on_pStartStop_toggled(bool bOn);
  void onTimeTrackingStopped(task_id id);
  void onPropertyEdited();
  void onAddPropertyTriggered();
  void updateSize();
  void updateSize2();
  void onDeleteTriggered();
  void onLinkPasted();
  void onAddSubtaskTriggered();

private:
  bool eventFilter(QObject* pObj, QEvent* pEvent) override;

  void setUpContextMenu();

private:
  void mousePressEvent(QMouseEvent* pMouseEvent) override;
  void mouseMoveEvent(QMouseEvent* pMouseEvent) override;
  void paintEvent(QPaintEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;
  void focusInEvent(QFocusEvent* pEvent) override;
  void focusOutEvent(QFocusEvent* pEvent) override;
  void enterEvent(QEvent* pEvent) override;
  void leaveEvent(QEvent* pEvent) override;
  void contextMenuEvent(QContextMenuEvent* pEvent) override;
  void showEvent(QShowEvent* pEvent) override;
  void dragEnterEvent(QDragEnterEvent* pEvent) override;
  void dropEvent(QDropEvent* pEvent) override;

  Ui::TaskWidget *ui;
  QPixmap m_cache;

  task_id m_taskId;

  bool m_bMouseDown = false;
  QPoint m_mouseDownPos;
  QImage m_backgroundImage[2];
  double m_dBackgroundImageBlendFactor = 0;
  bool m_bDropShadow = true;

  GroupWidget* m_pGroupWidget = nullptr;
  GroupWidget* m_pPreviousGroupWidget = nullptr;

  TaskWidgetOverlay* m_pOverlay;

  std::map<QString, EditableLabel*> m_propertyLineEdits;

  TaskWidget* m_pParentTask = nullptr;

  QMenu* m_pContextMenu = nullptr;

  std::set<TaskWidget*> m_subTasks;
  std::map<QUrl, LinkWidget*> m_linkWidgets;

  static TaskWidget* m_pDraggingTaskWidget;
  static TaskWidget* m_pTaskWidgetUnderMouse;
};

#endif // TASKWIDGET_H
