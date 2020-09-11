#ifndef REMOVETAGCOMMAND_H
#define REMOVETAGCOMMAND_H

#include "addtagcommand.h"

class RemoveTagCommand : public AddTagCommand
{
public:
  RemoveTagCommand(task_id taskId,
                   tag_id tagId,
                   Manager* pManager,
                   WidgetManager* pWidgetManager);

  void undo() override;

  void redo() override;
};

#endif // REMOVETAGCOMMAND_H
