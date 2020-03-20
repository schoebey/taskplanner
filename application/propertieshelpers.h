#ifndef PROPERTIESHELPERS_H
#define PROPERTIESHELPERS_H

#include "task.h"
#include "manager.h"
#include "widgetmanager.h"
#include "taskwidget.h"

#include "commands/changetaskpropertycommand.h"

#include <QString>
#include <QUndoStack>

namespace properties
{
template<typename T>
void onContainerElementAdded(Manager* pManager,
                             WidgetManager* pWidgetManager,
                             QUndoStack& undoStack,
                             task_id taskId,
                             const T& el,
                             const QString& sContainerName)
{
  ITask* pTask = pManager->task(taskId);
  if (nullptr != pTask)
  {
    bool bOk(false);
    auto elements = conversion::fromString<std::vector<T>>(pTask->propertyValue(sContainerName), bOk);
    if (bOk)
    {
      auto it = std::find(elements.begin(), elements.end(), el);
      if (it == elements.end())
      {
        elements.push_back(el);
        QUndoCommand* pCommand = new ChangeTaskPropertyCommand(taskId, sContainerName,
                                                               pTask->propertyValue(sContainerName),
                                                               conversion::toString(elements),
                                                               pManager, pWidgetManager);
        undoStack.push(pCommand);
      }
      else
      {
        // link already present - do nothing
      }
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    assert(false);
  }
}

template<typename T>
void onContainerElementRemoved(Manager* pManager,
                               WidgetManager* pWidgetManager,
                               QUndoStack& undoStack,
                               task_id taskId,
                               const T& el,
                               const QString& sContainerName)
{
  ITask* pTask = pManager->task(taskId);
  if (nullptr != pTask)
  {
    bool bOk(false);
    auto elements = conversion::fromString<std::vector<T>>(pTask->propertyValue(sContainerName), bOk);
    if (bOk)
    {
      auto it = std::find(elements.begin(), elements.end(), el);
      if (it != elements.end())
      {
        elements.erase(it);
        QUndoCommand* pCommand = new ChangeTaskPropertyCommand(taskId, sContainerName,
                                                               pTask->propertyValue(sContainerName),
                                                               conversion::toString(elements),
                                                               pManager, pWidgetManager);
        undoStack.push(pCommand);
      }
      else
      {
        // link not present - do nothing
      }
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    assert(false);
  }
}

bool onPropertyChanged(Manager* pManager,
                       WidgetManager* pWidgetManager,
                       QUndoStack& undoStack,
                       task_id taskId,
                       const QString& sPropertyName,
                       const QString& sValue)
{
  ITask* pTask = pManager->task(taskId);
  if (nullptr != pTask)
  {
    QString sOldValue = pTask->propertyValue(sPropertyName);
    if (sOldValue == sValue)  { return false; }

    bool bNewValueAccepted = pTask->setPropertyValue(sPropertyName, sValue);


    TaskWidget* pTaskWidget = pWidgetManager->taskWidget(taskId);
    if (nullptr != pTaskWidget)
    {
      if (bNewValueAccepted)
      {
        ChangeTaskPropertyCommand* pChangeCommand =
            new ChangeTaskPropertyCommand(taskId, sPropertyName, sOldValue, sValue, pManager, pWidgetManager);
        undoStack.push(pChangeCommand);
      }

      pTaskWidget->setHighlight(pTaskWidget->highlight() |
                               (bNewValueAccepted ? EHighlightMethod::eValueAccepted :
                                                   EHighlightMethod::eValueRejected));
      pTaskWidget->onPropertyValueChanged(sPropertyName, pTask->propertyValue(sPropertyName));

      if ("color" == sPropertyName)
      {
        bool bOk(false);
        auto color = conversion::fromString<QColor>(pTask->propertyValue("color"), bOk);
        if (bOk)
        {
          pTaskWidget->setOverlayBackground(color);
        }
      }
      else if ("priority" == sPropertyName)
      {
        pTaskWidget->setAutoPriority(pTask->autoPriority());
      }
    }

    return true;
  }

  return false;
}
}

#endif // PROPERTIESHELPERS_H
