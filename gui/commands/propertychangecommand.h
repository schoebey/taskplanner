#ifndef PROPERTYCHANGECOMMAND_H
#define PROPERTYCHANGECOMMAND_H

#include "propertyproviderinterface.h"
#include "taskwidget.h"

#include <QUndoCommand>

class PropertyChangeCommand : public QUndoCommand
{
public:
    PropertyChangeCommand(IPropertyProvider* pPropertyProvider,
                          IPropertyProvider* pPropertyProviderWidget,
                          const QString& sPropertyName,
                          const QString& sOldValue,
                          const QString& sNewValue);
    void undo() override;
    void redo() override;

private:
    IPropertyProvider* m_pPropertyProvider;
    IPropertyProvider* m_pPropertyProviderWidget;
    QString m_sPropertyName;
    QString m_sNewValue;
    QString m_sOldValue;
};

#endif // PROPERTYCHANGECOMMAND_H
