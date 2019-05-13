#include "deletetaskcommand.h"
#include "manager.h"
#include "taskinterface.h"

DeleteTaskCommand::DeleteTaskCommand(task_id taskId,
                                     Manager* pManager,
                                     WidgetManager* pWidgetManager)
  : AddTaskCommand(pManager->task(taskId)->group(),
                   QString(), QString(),
                   pManager, pWidgetManager)
{
  // taskId can only be set after the base class c'tor has been called
  // since the base initializes it with -1
  m_taskId = taskId;
  setText(QString("Delete task \"%1\"").arg(pManager->task(taskId)->name()));
}

void DeleteTaskCommand::undo()
{
  AddTaskCommand::redo();
}

void DeleteTaskCommand::redo()
{
  AddTaskCommand::undo();
}
