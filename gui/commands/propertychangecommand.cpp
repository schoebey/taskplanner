#include "propertychangecommand.h"

PropertyChangeCommand::PropertyChangeCommand(IPropertyProvider* pPropertyProvider,
                                             IPropertyProvider* pPropertyProviderWidget,
                                             const QString& sPropertyName,
                                             const QString& sOldValue,
                                             const QString& sNewValue)
: m_pPropertyProvider(pPropertyProvider),
  m_pPropertyProviderWidget(pPropertyProviderWidget),
  m_sPropertyName(sPropertyName),
  m_sNewValue(sNewValue),
  m_sOldValue(sOldValue)
{
  setText(QString("change property '%1'").arg(m_sPropertyName));
}

void PropertyChangeCommand::undo()
{
  m_pPropertyProvider->setPropertyValue(m_sPropertyName, m_sOldValue);
  m_pPropertyProviderWidget->setPropertyValue(m_sPropertyName, m_pPropertyProvider->propertyValue(m_sPropertyName));
}

void PropertyChangeCommand::redo()
{
  m_pPropertyProvider->setPropertyValue(m_sPropertyName, m_sNewValue);
  m_pPropertyProviderWidget->setPropertyValue(m_sPropertyName, m_pPropertyProvider->propertyValue(m_sPropertyName));
}
