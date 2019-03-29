#ifndef ADDSUBTASKCOMMAND_H
#define ADDSUBTASKCOMMAND_H

#include "id_types.h"

#include <QString>
#include <QUndoCommand>

class Manager;
class WidgetManager;

class AddSubTaskCommand : public QUndoCommand
{
public:
  AddSubTaskCommand(task_id parentTaskId,
                    task_id childTaskId,
                    Manager* pManager,
                    WidgetManager* pWidgetManager);

  void undo() override;

  void redo() override;

private:
  task_id m_parentTaskId;
  task_id m_prevParentTaskId;
  int m_iPrevPos;
  task_id m_childTaskId;
  Manager* m_pManager;
  WidgetManager* m_pWidgetManager;
};

#endif // ADDSUBTASKCOMMAND_H
