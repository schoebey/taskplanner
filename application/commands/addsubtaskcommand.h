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
                    const QString& sName,
                    const QString& sDescription,
                    Manager* pManager,
                    WidgetManager* pWidgetManager);

  void undo() override;

  void redo() override;

private:
  task_id m_taskId;
  task_id m_parentTaskId;
  std::map<QString, QString> m_properties;
  Manager* m_pManager;
  WidgetManager* m_pWidgetManager;
};

#endif // ADDSUBTASKCOMMAND_H
