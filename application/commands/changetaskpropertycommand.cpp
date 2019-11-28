#include "changetaskpropertycommand.h"
#include "manager.h"
#include "widgetmanager.h"

#include "taskinterface.h"
#include "taskwidget.h"

namespace {
  void setProperty(task_id taskId, const QString& sProperty, const QString& sValue,
                   Manager* pManager, WidgetManager* pWidgetManager)
  {
    auto pTask = pManager->task(taskId);
    auto pTaskWidget = pWidgetManager->taskWidget(taskId);

    if (sValue.isEmpty())
    {
      if (nullptr != pTask) { pTask->removeProperty(sProperty); }
      if (nullptr != pTaskWidget) { pTaskWidget->removeProperty(sProperty); }
    }
    else
    {
      if (nullptr != pTask) { pTask->setPropertyValue(sProperty, sValue); }
      if (nullptr != pTaskWidget) { pTaskWidget->onPropertyValueChanged(sProperty, pTask->propertyValue(sProperty)); }
    }
  }
}

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
  setProperty(m_taskId, m_sPropertyName, m_sOldValue, m_pManager, m_pWidgetManager);
}

void ChangeTaskPropertyCommand::redo()
{
  setProperty(m_taskId, m_sPropertyName, m_sNewValue, m_pManager, m_pWidgetManager);
}
