#ifndef PROPERTIESHELPERS_H
#define PROPERTIESHELPERS_H

#include "task.h"
#include "manager.h"
#include "widgetmanager.h"
#include "taskwidget.h"

#include "commands/changetaskpropertycommand.h"
#include "commands/combinedundocommand.h"

#include <QString>
#include <QUndoStack>


namespace properties
{
template<typename T>
void onContainerElementInserted(Manager* pManager,
                                WidgetManager* pWidgetManager,
                                QUndoStack& undoStack,
                                task_id taskId,
                                const T& el,
                                const QString& sContainerName,
                                int iPosition)
{
  ITask* pTask = pManager->task(taskId);
  if (nullptr != pTask)
  {
    bool bOk(false);
    auto elements = conversion::fromString<std::vector<T>>(pTask->propertyValue(sContainerName), bOk);
    if (bOk)
    {
      auto it = std::find(elements.begin(), elements.end(), el);
      if (it == elements.end() && iPosition < static_cast<int>(elements.size()) && iPosition >= -1)
      {
        typename std::vector<T>::iterator itPosition;
        if (-1 == iPosition)
        {
          itPosition = elements.end();
        }
        else
        {
          itPosition = elements.begin() + iPosition;
        }
        //elements.insert(el, itPosition);
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
void onContainerElementMovedFrom(Manager* pManager,
                                 WidgetManager* pWidgetManager,
                                 QUndoStack& undoStack,
                                 task_id taskId,
                                 const T& el,
                                 const QString& sContainerName,
                                 task_id sourceTaskId)
{
  // for now, since we don't support reordering, do nothing
  // if task ids are identical
  if (taskId == sourceTaskId)  { return; }

  QUndoCommand* pCommand1 = nullptr;
  QUndoCommand* pCommand2 = nullptr;

  // target task
  ITask* pTask = pManager->task(taskId);
  if (nullptr != pTask)
  {

    bool bOk(false);
    auto elements = conversion::fromString<std::vector<T>>(pTask->propertyValue(sContainerName), bOk);
    if (bOk)
    {
      elements.push_back(el);

      // TODO: insert at appropriate position
      pCommand1 = new ChangeTaskPropertyCommand(taskId, sContainerName,
                                                pTask->propertyValue(sContainerName),
                                                conversion::toString(elements),
                                                pManager, pWidgetManager);
    }
  }


  // source task
  ITask* pSource = pManager->task(sourceTaskId);
  if (nullptr != pSource)
  {
    bool bOk(false);
    auto elements = conversion::fromString<std::vector<T>>(pSource->propertyValue(sContainerName), bOk);
    if (bOk)
    {
      auto it = std::find(elements.begin(), elements.end(), el);
      if (it != elements.end())
      {
        elements.erase(it);
        pCommand2 = new ChangeTaskPropertyCommand(sourceTaskId, sContainerName,
                                                  pSource->propertyValue(sContainerName),
                                                  conversion::toString(elements),
                                                  pManager, pWidgetManager);
      }
      else
      {
        // link not present - should not happen since the element was moved from this task
        assert(false && "source does not contain element");
      }
    }
  }


  QUndoCommand* pCommand = new CombinedUndoCommand(pCommand1, pCommand2);
  undoStack.push(pCommand);
}

template<typename T>
void onContainerElementAdded(Manager* pManager,
                             WidgetManager* pWidgetManager,
                             QUndoStack& undoStack,
                             task_id taskId,
                             const T& el,
                             const QString& sContainerName)
{
  onContainerElementInserted(pManager, pWidgetManager, undoStack, taskId, el, sContainerName, -1);
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
