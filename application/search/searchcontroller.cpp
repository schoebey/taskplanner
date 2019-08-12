#include "searchcontroller.h"
#include "manager.h"
#include "widgetmanager.h"
#include "groupwidget.h"
#include "taskwidget.h"

namespace {
  void highlight(TaskWidget* pWidget, EHighlightMethod method)
  {
    if (nullptr != pWidget)
    {
      pWidget->setHighlight(method);
    }
  }

  void findRecursive(TaskWidget* pWidget,
                     SearchController::tvHitType& vpHits,
                     const QString& sSearchTerm)
  {
    if (pWidget->name().contains(sSearchTerm))
    {
      vpHits.push_back(pWidget);
    }

    for (const auto& pChild : pWidget->tasks())
    {
      findRecursive(pChild, vpHits, sSearchTerm);
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

  for (auto& el :m_vpHits)
  {
    highlight(el, eNoHighlight);
  }

  m_vpHits.clear();
  if (!sTerm.isEmpty())
  {
    for (const auto& groupId : m_pTaskManager->groupIds())
    {
      auto pGroupWidget = m_pWidgetManager->groupWidget(groupId);
      if (nullptr != pGroupWidget)
      {
        for (const auto& pTask : pGroupWidget->tasks())
        {
          if (nullptr != pTask)
          {
            findRecursive(pTask, m_vpHits, sTerm);
          }
        }
      }
    }
  }

  for (auto& el :m_vpHits)
  {
    highlight(el, eSearchResult);
  }

  m_hitIter = m_vpHits.end();
  auto it = std::find(m_vpHits.begin(), m_vpHits.end(), pCurrentHit);
  if (m_vpHits.end() != it)
  {
    setCurrent(it);
  }
  else
  {
    setCurrent(m_vpHits.begin());
  }
}

void SearchController::setCurrent(tvHitType::iterator it)
{
  if (m_hitIter != m_vpHits.end())
  {
    highlight(*m_hitIter, eSearchResult);
  }

  m_hitIter = it;
  if (m_hitIter != m_vpHits.end())
  {
    highlight(*m_hitIter, eActiveSearchResult);
    emit positionChanged(static_cast<size_t>(m_hitIter - m_vpHits.begin()), hitCount());
  }
  else
  {
    highlight(nullptr, eActiveSearchResult);
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
  else
  {
    setCurrent(m_vpHits.begin());
  }
}

void SearchController::onPrev()
{
  if (m_hitIter != m_vpHits.end() &&
      m_hitIter != m_vpHits.begin())
  {
    setCurrent(m_hitIter - 1);
  }
  else if (!m_vpHits.empty())
  {
    setCurrent(m_vpHits.end() - 1);
  }
}
