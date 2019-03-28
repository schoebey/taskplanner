#ifndef PROPERTYCHANGECOMMAND_H
#define PROPERTYCHANGECOMMAND_H

#include "id_types.h"

#include <QUndoCommand>

class Manager;
class WidgetManager;
class ChangeTaskPropertyCommand : public QUndoCommand
{
public:
    ChangeTaskPropertyCommand(task_id taskId,
                          const QString& sPropertyName,
                          const QString& sOldValue,
                          const QString& sNewValue,
                          Manager* pManager,
                          WidgetManager* pWidgetManager);
    void undo() override;
    void redo() override;

private:
    task_id m_taskId;
    QString m_sPropertyName;
    QString m_sNewValue;
    QString m_sOldValue;
    Manager* m_pManager;
    WidgetManager* m_pWidgetManager;
};

#endif // PROPERTYCHANGECOMMAND_H
