#ifndef ADDTAGCOMMAND_H
#define ADDTAGCOMMAND_H

#include "id_types.h"

#include <QString>
#include <QUndoCommand>

#include <map>

class Manager;
class WidgetManager;
class AddTagCommand : public QUndoCommand
{
public:
  AddTagCommand(task_id taskId,
                tag_id tagId,
                Manager* pManager,
                WidgetManager* pWidgetManager);

  void undo() override;

  void redo() override;

protected:
  task_id m_taskId;
  tag_id m_tagId;
  Manager* m_pManager;
  WidgetManager* m_pWidgetManager;
};
#endif // ADDTAGCOMMAND_H
