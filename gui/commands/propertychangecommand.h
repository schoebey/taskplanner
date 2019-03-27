#ifndef PROPERTYCHANGECOMMAND_H
#define PROPERTYCHANGECOMMAND_H

#include "taskinterface.h"
#include "taskwidget.h"

#include <QUndoCommand>

class PropertyChangeCommand : public QUndoCommand
{
public:
    PropertyChangeCommand(ITask* pTask,
                          TaskWidget* pTaskWidget,
                          const QString& sPropertyName,
                          const QString& sOldValue,
                          const QString& sNewValue);
    void undo() override;
    void redo() override;

private:
    ITask* m_pTask;
    TaskWidget* m_pTaskWidget;
    QString m_sPropertyName;
    QString m_sNewValue;
    QString m_sOldValue;
};

#endif // PROPERTYCHANGECOMMAND_H
