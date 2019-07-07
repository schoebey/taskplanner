#ifndef ITASKCONTAINERWIDGET_H
#define ITASKCONTAINERWIDGET_H

class TaskWidget;
class ITaskContainerWidget
{
public:
  virtual void requestInsert(TaskWidget* pTaskWidget, int iPos = -1) = 0;
  virtual bool insertTask(TaskWidget* pTaskWidget, int iPos = -1) = 0;
  virtual void removeTask(TaskWidget* pTaskWidget) = 0;
};

#endif // ITASKCONTAINERWIDGET_H
