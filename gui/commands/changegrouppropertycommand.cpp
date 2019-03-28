#include "changegrouppropertycommand.h"
#include "manager.h"
#include "widgetmanager.h"

#include "groupinterface.h"
#include "groupwidget.h"

ChangeGroupPropertyCommand::ChangeGroupPropertyCommand(group_id groupId,
                                                       const QString& sPropertyName,
                                                       const QString& sOldValue,
                                                       const QString& sNewValue,
                                                       Manager* pManager,
                                                       WidgetManager* pWidgetManager)
: m_groupId(groupId),
  m_sPropertyName(sPropertyName),
  m_sNewValue(sNewValue),
  m_sOldValue(sOldValue),
  m_pManager(pManager),
  m_pWidgetManager(pWidgetManager)
{
  setText(QString("change property '%1'").arg(m_sPropertyName));
}

void ChangeGroupPropertyCommand::undo()
{
  auto pGroup = m_pManager->group(m_groupId);
  if (nullptr != pGroup) { pGroup->setPropertyValue(m_sPropertyName, m_sOldValue); }

  auto pGroupWidget = m_pWidgetManager->groupWidget(m_groupId);
  if (nullptr != pGroupWidget) { pGroupWidget->setPropertyValue(m_sPropertyName, m_sOldValue); }
}

void ChangeGroupPropertyCommand::redo()
{
  auto pGroup = m_pManager->group(m_groupId);
  if (nullptr != pGroup) { pGroup->setPropertyValue(m_sPropertyName, m_sNewValue); }

  auto pGroupWidget = m_pWidgetManager->groupWidget(m_groupId);
  if (nullptr != pGroupWidget) { pGroupWidget->setPropertyValue(m_sPropertyName, m_sNewValue); }
}
