#ifndef MOVETASKCOMMAND_H
#define MOVETASKCOMMAND_H

#include <QUndoCommand>

class ITask;
class IGroup;
class Manager;
class TaskWidget;
class GroupWidget;

class MoveTaskCommand : public QUndoCommand
{
public:
    MoveTaskCommand(ITask* pTask, TaskWidget* pTaskWidget,
                    IGroup* pOldGroup, GroupWidget* pOldGroupWidget,
                    IGroup* pNewGroup, GroupWidget* pNewGroupWidget,
                    int iOldPosition,
                    int iNewPosition,
                    Manager* pManager);
    void undo() override;
    void redo() override;

private:
    ITask* m_pTask;
    TaskWidget* m_pTaskWidget;
    IGroup* m_pOldGroup;
    GroupWidget* m_pOldGroupWidget;
    IGroup* m_pNewGroup;
    GroupWidget* m_pNewGroupWidget;
    int m_iOldPosition;
    int m_iNewPosition;
    Manager* m_pManager;
};

#endif // MOVETASKCOMMAND_H
