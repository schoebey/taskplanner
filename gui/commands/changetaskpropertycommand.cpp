#include "changetaskpropertycommand.h"
#include "manager.h"
#include "widgetmanager.h"

#include "taskinterface.h"
#include "taskwidget.h"

ChangeTaskPropertyCommand::ChangeTaskPropertyCommand(task_id taskId,
                                             const QString& sPropertyName,
                                             const QString& sOldValue,
                                             const QString& sNewValue,
                                             Manager* pManager,
                                             WidgetManager* pWidgetManager)
: m_taskId(taskId),
  m_sPropertyName(sPropertyName),
  m_sNewValue(sNewValue),
  m_sOldValue(sOldValue),
  m_pManager(pManager),
  m_pWidgetManager(pWidgetManager)
{
  setText(QString("change property '%1'").arg(m_sPropertyName));
}

void ChangeTaskPropertyCommand::undo()
{
  auto pTask = m_pManager->task(m_taskId);
  if (nullptr != pTask) { pTask->setPropertyValue(m_sPropertyName, m_sOldValue); }

  auto pTaskWidget = m_pWidgetManager->taskWidget(m_taskId);
  if (nullptr != pTaskWidget) { pTaskWidget->setPropertyValue(m_sPropertyName, m_sOldValue); }
}

void ChangeTaskPropertyCommand::redo()
{
  auto pTask = m_pManager->task(m_taskId);
  if (nullptr != pTask) { pTask->setPropertyValue(m_sPropertyName, m_sNewValue); }

  auto pTaskWidget = m_pWidgetManager->taskWidget(m_taskId);
  if (nullptr != pTaskWidget) { pTaskWidget->setPropertyValue(m_sPropertyName, m_sNewValue); }
}
