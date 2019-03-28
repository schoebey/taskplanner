#ifndef CHANGEGROUPPROPERTYCOMMAND_H
#define CHANGEGROUPPROPERTYCOMMAND_H

#include "id_types.h"

#include <QUndoCommand>

class Manager;
class WidgetManager;
class ChangeGroupPropertyCommand : public QUndoCommand
{
public:
    ChangeGroupPropertyCommand(group_id groupId,
                          const QString& sPropertyName,
                          const QString& sOldValue,
                          const QString& sNewValue,
                          Manager* pManager,
                          WidgetManager* pWidgetManager);
    void undo() override;
    void redo() override;

private:
    group_id m_groupId;
    QString m_sPropertyName;
    QString m_sNewValue;
    QString m_sOldValue;
    Manager* m_pManager;
    WidgetManager* m_pWidgetManager;
};


#endif // CHANGEGROUPPROPERTYCOMMAND_H
