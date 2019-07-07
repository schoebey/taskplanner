#ifndef MOVETASKCOMMAND_H
#define MOVETASKCOMMAND_H

#include "id_types.h"
#include <QUndoCommand>

class Manager;
class WidgetManager;

class MoveTaskCommand : public QUndoCommand
{
public:
    MoveTaskCommand(task_id taskId,
                    group_id oldGroupId,
                    group_id newGroupId,
                    task_id oldParentTaskId,
                    task_id newParentTaskId,
                    int iOldPosition,
                    int iNewPosition,
                    Manager* pManager,
                    WidgetManager* pWidgetManager);
    virtual ~MoveTaskCommand();

    void undo() override;
    void redo() override;

private:
    task_id m_taskId;
    group_id m_oldGroupId;
    group_id m_newGroupId;
    task_id m_oldParentTaskId;
    task_id m_newParentTaskId;
    int m_iOldPosition;
    int m_iNewPosition;
    Manager* m_pManager;
    WidgetManager* m_pWidgetManager;
};

#endif // MOVETASKCOMMAND_H
