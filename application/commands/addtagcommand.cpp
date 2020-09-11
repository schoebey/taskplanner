#include "addtagcommand.h"
#include "manager.h"
#include "widgetmanager.h"
#include "taskinterface.h"
#include "taginterface.h"
#include "taskwidget.h"
#include "tagwidget.h"

#include <cassert>

AddTagCommand::AddTagCommand(task_id taskId,
                             tag_id tagId,
                             Manager* pManager,
                             WidgetManager* pWidgetManager)
  : m_taskId(taskId),
    m_tagId(tagId),
    m_pManager(pManager),
    m_pWidgetManager(pWidgetManager)
{
  setText(QString("add tag \"%1\"").arg(pManager->tag(tagId)->name()));
}

void AddTagCommand::undo()
{
  // add the tag id to the task in the manager
  ITask* pTask = m_pManager->task(m_taskId);
  if (nullptr != pTask && pTask->removeTag(m_tagId))
  {
    // add the tag widget to the task widget
    TaskWidget* pTaskWidget = m_pWidgetManager->taskWidget(m_taskId);
    if (nullptr != pTaskWidget)
    {
      // create the tag widget
      for (const auto& el : pTaskWidget->tags()) {
        if (nullptr != el &&
            el->id() == m_tagId &&
            pTaskWidget->removeTag(el))
        {
          delete el;
          break;
        }
      }
    }
  }
}

void AddTagCommand::redo()
{
  // add the tag id to the task in the manager
  ITask* pTask = m_pManager->task(m_taskId);
  if (nullptr != pTask && pTask->addTag(m_tagId))
  {
    // add the tag widget to the task widget
    TaskWidget* pTaskWidget = m_pWidgetManager->taskWidget(m_taskId);
    if (nullptr != pTaskWidget)
    {
      // create the tag widget
      TagWidget* pTagWidget = m_pWidgetManager->createTagWidget(m_tagId);

      pTaskWidget->addTag(pTagWidget);
    }
  }
}
