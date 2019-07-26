#ifndef ADDTASKCOMMAND_H
#define ADDTASKCOMMAND_H

#include "id_types.h"

#include <QString>
#include <QUndoCommand>

#include <map>

class Manager;
class WidgetManager;
class AddTaskCommand : public QUndoCommand
{
public:
    AddTaskCommand(group_id groupId,
                   task_id parentTaskId,
                   const QString& sName,
                   const QString& sDescription,
                   Manager* pManager,
                   WidgetManager* pWidgetManager);

    void undo() override;

    void redo() override;

protected:
    task_id m_taskId;
    group_id m_groupId;
    task_id m_parentTaskId;
    std::map<QString, QString> m_properties;
    Manager* m_pManager;
    WidgetManager* m_pWidgetManager;
};
#endif // ADDTASKCOMMAND_H
