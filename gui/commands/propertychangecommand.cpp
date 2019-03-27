#include "propertychangecommand.h"

PropertyChangeCommand::PropertyChangeCommand(ITask* pTask,
                                             TaskWidget* pTaskWidget,
                                             const QString& sPropertyName,
                                             const QString& sOldValue,
                                             const QString& sNewValue)
: m_pTask(pTask),
  m_pTaskWidget(pTaskWidget),
  m_sPropertyName(sPropertyName),
  m_sNewValue(sNewValue),
  m_sOldValue(sOldValue)
{
  setText(QString("change property '%1'").arg(m_sPropertyName));
}

void PropertyChangeCommand::undo()
{
  m_pTask->setPropertyValue(m_sPropertyName, m_sOldValue);
  m_pTaskWidget->setPropertyValue(m_sPropertyName, m_sOldValue);
}

void PropertyChangeCommand::redo()
{
  m_pTask->setPropertyValue(m_sPropertyName, m_sNewValue);
  m_pTaskWidget->setPropertyValue(m_sPropertyName, m_sNewValue);
}
