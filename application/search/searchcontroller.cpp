#include "searchcontroller.h"
#include "manager.h"
#include "widgetmanager.h"
#include "groupwidget.h"
#include "taskwidget.h"

namespace {
  void highlight(TaskWidget* pWidget, bool bSetHighlight = true)
  {
    if (nullptr != pWidget)
    {
      // TODO
    }
  }
}

SearchController::SearchController(Manager* pManager,
                                   WidgetManager* pWidgetManager)
  : QObject(),
    m_pTaskManager(pManager),
    m_pWidgetManager(pWidgetManager),
    m_hitIter(m_vpHits.end())
{
}

SearchController::~SearchController()
{

}

size_t SearchController::hitCount() const
{
  return m_vpHits.size();
}

void SearchController::onSearchTermChanged(const QString& sTerm)
{
  TaskWidget* pCurrentHit = nullptr;
  if (m_hitIter != m_vpHits.end())
  {
    pCurrentHit = *m_hitIter;
  }
  highlight(pCurrentHit, false);

  m_vpHits.clear();
  m_hitIter = m_vpHits.end();
  for (const auto& groupId : m_pTaskManager->groupIds())
  {
    auto pGroupWidget = m_pWidgetManager->groupWidget(groupId);
    if (nullptr != pGroupWidget)
    {
      for (const auto& pTask : pGroupWidget->tasks())
      {
        if (nullptr != pTask)
        {
          if (pTask->name().contains(sTerm))
          {
            m_vpHits.push_back(pTask);
          }
        }
      }
    }
  }

  setCurrent(std::find(m_vpHits.begin(), m_vpHits.end(), pCurrentHit));
}

void SearchController::setCurrent(tvHitType::iterator it)
{
  if (m_hitIter != m_vpHits.end())
  {
    highlight(*m_hitIter, false);
  }

  m_hitIter = it;
  if (m_hitIter != m_vpHits.end())
  {
    highlight(*m_hitIter);
    emit positionChanged(static_cast<size_t>(m_hitIter - m_vpHits.begin()), hitCount());
  }
  else
  {
    highlight(nullptr);
    emit positionChanged(0, hitCount());
  }
}

void SearchController::onNext()
{
  if (m_hitIter != m_vpHits.end() &&
      m_hitIter != m_vpHits.end() - 1)
  {
    setCurrent(m_hitIter + 1);
  }
}

void SearchController::onPrev()
{
  if (m_hitIter != m_vpHits.end() &&
      m_hitIter != m_vpHits.begin())
  {
    setCurrent(m_hitIter - 1);
  }
}
