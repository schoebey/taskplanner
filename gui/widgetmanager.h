#ifndef WIDGETSMANAGER_H
#define WIDGETSMANAGER_H

#include "id_types.h"
#include <QPointer>

class TaskWidget;
class GroupWidget;
class Manager;

class WidgetManager
{
public:
  WidgetManager(Manager* pManager, QWidget* pController);

  void clear();

  TaskWidget* taskWidget(task_id id) const;
  GroupWidget* groupWidget(group_id id) const;
  GroupWidget* createGroupWidget(group_id id);
  TaskWidget* createTaskWidget(task_id id);
  bool deleteTaskWidget(task_id id);
  bool deleteGroupWidget(group_id id);

private:
  Manager* m_pManager;
  QWidget* m_pController;
  std::map<group_id, QPointer<GroupWidget>> m_groupWidgets;
  std::map<task_id, QPointer<TaskWidget>> m_taskWidgets;
};

#endif // WIDGETSMANAGER_H
