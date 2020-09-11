#include "removetagcommand.h"
#include "manager.h"
#include "taginterface.h"

RemoveTagCommand::RemoveTagCommand(task_id taskId,
                                   tag_id tagId,
                                   Manager* pManager,
                                   WidgetManager* pWidgetManager)
  : AddTagCommand(taskId, tagId,
                   pManager, pWidgetManager)
{
  // taskId can only be set after the base class c'tor has been called
  // since the base initializes it with -1
  m_taskId = taskId;
  setText(QString("Remove tag  \"%1\"").arg(pManager->tag(tagId)->name()));
}

void RemoveTagCommand::undo()
{
  AddTagCommand::redo();
}

void RemoveTagCommand::redo()
{
  AddTagCommand::undo();
}

