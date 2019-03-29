#ifndef DELETETASKCOMMAND_H
#define DELETETASKCOMMAND_H

#include "addtaskcommand.h"

class DeleteTaskCommand : public AddTaskCommand
{
public:
  DeleteTaskCommand(task_id taskId,
                    Manager* pManager,
                    WidgetManager* pWidgetManager);

  void undo() override;

  void redo() override;
};

#endif // DELETETASKCOMMAND_H
