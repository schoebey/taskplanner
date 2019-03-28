#ifndef MOVETASKCOMMAND_H
#define MOVETASKCOMMAND_H

#include "id_types.h"
#include <QUndoCommand>
;
class Manager;
class WidgetManager;

class MoveTaskCommand : public QUndoCommand
{
public:
    MoveTaskCommand(task_id taskId,
                    group_id oldGroupId,
                    group_id newGroupId,
                    int iOldPosition,
                    int iNewPosition,
                    Manager* pManager,
                    WidgetManager* pWidgetManager);
    void undo() override;
    void redo() override;

private:
    task_id m_taskId;
    group_id m_oldGroupId;
    group_id m_newGroupId;
    int m_iOldPosition;
    int m_iNewPosition;
    Manager* m_pManager;
    WidgetManager* m_pWidgetManager;
};

#endif // MOVETASKCOMMAND_H
