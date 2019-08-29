#ifndef ITASKCONTAINERWIDGET_H
#define ITASKCONTAINERWIDGET_H

#include <vector>

class QWidget;
class TaskWidget;
class ITaskContainerWidget
{
public:
  virtual void requestInsert(TaskWidget* pTaskWidget, int iPos = -1) = 0;
  virtual bool insertTask(TaskWidget* pTaskWidget, int iPos = -1) = 0;
  virtual void removeTask(TaskWidget* pTaskWidget) = 0;
  virtual std::vector<TaskWidget*> tasks() const = 0;
  virtual void ensureVisible(QWidget* pWidget) = 0;
};

#endif // ITASKCONTAINERWIDGET_H
