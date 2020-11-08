#include "searchcontroller.h"
#include "manager.h"
#include "widgetmanager.h"
#include "groupwidget.h"
#include "taskwidget.h"
#include "taskinterface.h"


#include <QRegularExpression>
#include <QLabel>

#include <deque>

/*

  instead of just storing pointers to task widgets, store additional information
  such as the text label that contains the hit and its hit index (start/end index).

  this way, the label can pass this information to its draw call which in turn
  can highlight the passage in question.

  hit type: struct { label*, start_index, count}

  ensure_visible: climbs up to closest taskwidget, proceeds normally from there


  struct SHighlight
  {
    int start;
    int count;
  };
  register metatype

  when searching, either delegate the search to the task widget, or get a list
  of labels from it and search "manually".

  hitType:
  * QRect
  * start index
  * hit size
  std::map<QWidget*, std::vector<tHitType>>  search(term, widget)
  {
    for all child labels of widget:
    determine start and size indexes for all hits within label
    call style for a list of rects from that list
    set the vector of hits as a property of the label
  }
*/

namespace detail
{
  void ensureVisible(QWidget* pWidget, QWidget* pOriginal)
  {
    if (nullptr == pWidget)  { return; }
    TaskWidget* pTaskWidget = dynamic_cast<TaskWidget*>(pWidget);
    if (nullptr == pTaskWidget)
    {
      ensureVisible(pWidget->parentWidget(), pOriginal);
    }
    else
    {
      pTaskWidget->ensureVisible(pOriginal);
    }
  }

  void ensureVisible(QWidget* pWidget)
  {
    ensureVisible(pWidget, pWidget);
  }

  void highlight(QWidget* pWidget, const tvMatchInfo& vMatches,
                 EHighlightMethod method = EHighlightMethod::eSearchResult)
  {
    if (nullptr != pWidget)
    {
      switch (method)
      {
      case EHighlightMethod::eSearchResult:
        pWidget->setProperty("highlights", QVariant::fromValue(vMatches));
        break;
      case EHighlightMethod::eActiveSearchResult:
        pWidget->setProperty("active", QVariant::fromValue(vMatches));
        break;
      default:
        break;
      }
      pWidget->update();
    }
  }

  void addMatch(SearchController::tMatches& matches,
                QLabel* pLabel,
                int iIndex,
                int iSize)
  {

    SMatchInfo matchType;
    matchType.pWidget = pLabel;
    matchType.iStart = iIndex;
    matchType.iSize = iSize;
    matches.push_back(matchType);
  }

  struct SSearchResult
  {
    int iIndex = -1;
    int iSize = -1;
  };

  SSearchResult findRx(const QString& sTerm,
                       const QString& s,
                       Qt::CaseSensitivity cs,
                       int iIndex)
  {
    QRegExp rx(sTerm, cs);
    if (-1 != (iIndex = rx.indexIn(s, iIndex + 1)))
    {
      return SSearchResult{iIndex, rx.cap().size()};
    }

    return SSearchResult{};
  }

  SSearchResult findStr(const QString& sTerm,
                        const QString& s,
                        Qt::CaseSensitivity cs,
                        int iIndex)
  {
    if (-1 != (iIndex = s.indexOf(sTerm, iIndex + 1, cs)))
    {
      return SSearchResult{iIndex, sTerm.size()};
    }

    return SSearchResult{};
  }

  void find(const QString& sTerm,
            const QList<QLabel*>& labels,
            Qt::CaseSensitivity cs,
            SearchController::tMatches& matches,
            std::function<SSearchResult(const QString&, const QString&, Qt::CaseSensitivity, int)> fnMatch)
  {
    for (const auto pLabel : labels)
    {
      SSearchResult sr = fnMatch(sTerm, pLabel->text(), cs, -1);
      while (sr.iIndex != -1)
      {
        addMatch(matches, pLabel, sr.iIndex, sr.iSize);
        sr = fnMatch(sTerm, pLabel->text(), cs, sr.iIndex + 1);
      }
    }
  }
}

namespace {
  void find(const QString& sTerm, const QWidget* pWidget,
            bool bCaseSensitive, bool bRegExp,
            SearchController::tMatches& matches)
  {
    auto labels = pWidget->findChildren<QLabel*>();
    std::sort(labels.begin(), labels.end(), [pWidget](const QLabel* pLhs, const QLabel* pRhs)
    { return pLhs->mapTo(pWidget, pLhs->pos()).y() < pRhs->mapTo(pWidget, pRhs->pos()).y(); });

    Qt::CaseSensitivity cs = bCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    if (bRegExp)
    {
      detail::find(sTerm, labels, cs, matches, detail::findRx);
    }
    else
    {
      detail::find(sTerm, labels, cs, matches, detail::findStr);
    }
  }

  bool containsMatch(const QString& sTerm, const ITask* pTask,
                     bool bCaseSensitive, bool bRegExp)
  {

    Qt::CaseSensitivity cs = bCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    pTask->name();
    pTask->description();

    std::function<detail::SSearchResult(const QString&, const QString&, Qt::CaseSensitivity, int)> fnFind
        = bRegExp ? detail::findStr : detail::findRx;

    detail::SSearchResult sr;
    sr = fnFind(pTask->name(), sTerm, cs, -1);
    if (sr.iIndex != -1)  { return true; }
    sr = fnFind(pTask->description(), sTerm, cs, -1);
    if (sr.iIndex != -1)  { return true; }

    return false;
  }
}

SearchController::SearchController(Manager* pManager,
                                   WidgetManager* pWidgetManager)
  : QObject(),
    m_pTaskManager(pManager),
    m_pWidgetManager(pWidgetManager),
    m_hitIter(m_hits.end())
{
}

SearchController::~SearchController()
{

}

size_t SearchController::hitCount() const
{
  return m_hits.size();
}

void SearchController::onSearchTermChanged(const QString& sTerm)
{
  m_sSearchTerm = sTerm;
  QWidget* pCurrentWidget = nullptr;
  int iCurrentIndex = -1;
  if (m_hitIter != m_hits.end())
  {
    pCurrentWidget = m_hitIter->pWidget;
    iCurrentIndex = m_hitIter->iStart;
  }

  std::map<QWidget*, tvMatchInfo> matchesByWidget;
  for (auto& el : m_hits)
  {
    matchesByWidget[el.pWidget].push_back(el);
  }

  for (const auto& el : matchesByWidget)
  {
    detail::highlight(el.first, tvMatchInfo{}, eActiveSearchResult);
    detail::highlight(el.first, tvMatchInfo{}, eSearchResult);
  }

  m_hits.clear();
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
            find(sTerm, pTask, m_options.bCaseSensitive, m_options.bRegularExpression, m_hits);
          }
        }
      }
    }
  }


  matchesByWidget.clear();
  for (auto& el : m_hits)
  {
    matchesByWidget[el.pWidget].push_back(el);
  }

  for (const auto& el : matchesByWidget)
  {
    detail::highlight(el.first, el.second, eSearchResult);
  }

  m_hitIter = m_hits.end();
  auto it = std::find_if(m_hits.begin(), m_hits.end(),
                         [pCurrentWidget, iCurrentIndex](const tMatches::value_type& el )
  {
    return el.pWidget == pCurrentWidget && el.iStart == iCurrentIndex;
  });

  if (m_hits.end() != it)
  {
    setCurrent(it);
  }
  else
  {
    setCurrent(m_hits.begin());
  }
}

void SearchController::setCurrent(tMatches::iterator it)
{
  if (m_hitIter != m_hits.end())
  {
    // clear the active highlight
    detail::highlight(m_hitIter->pWidget, tvMatchInfo{}, eActiveSearchResult);
  }

  m_hitIter = it;
  if (m_hitIter != m_hits.end())
  {
    QWidget* pCurrent = m_hitIter->pWidget;
    detail::ensureVisible(pCurrent);
    detail::highlight(m_hitIter->pWidget, tvMatchInfo{*m_hitIter}, eActiveSearchResult);
    emit positionChanged(static_cast<size_t>(m_hitIter - m_hits.begin()), hitCount());
  }
  else
  {
    emit positionChanged(0, hitCount());
  }
}

void SearchController::onNext()
{
  auto it = m_hitIter;
  if (m_hitIter != m_hits.end() &&
      m_hitIter != m_hits.end() - 1)
  {
    it = m_hitIter + 1;
  }
  else
  {
    it = m_hits.begin();
  }

  // advance the iterator from this position to the next valid one,
  // removing all invalid elements inbetween.
  while (nullptr == it->pWidget)
  {
    it = m_hits.erase(it);
  }

  setCurrent(it);
}

void SearchController::onPrev()
{
  auto it = m_hitIter;
  if (m_hitIter != m_hits.end() &&
      m_hitIter != m_hits.begin())
  {
    it = m_hitIter - 1;
  }
  else if (!m_hits.empty())
  {
    it = m_hits.end() - 1;
  }

  // reverse the iterator from this position to the previous valid one,
  // removing all invalid elements inbetween.
  while (nullptr == it->pWidget)
  {
    it = m_hits.erase(it) - 1;
  }

  setCurrent(it);
}

void SearchController::onSearchOptionsChanged(const SearchOptions& options)
{
  setOptions(options);
  onSearchTermChanged(m_sSearchTerm);
}

void SearchController::setOptions(const SearchOptions& options)
{
  m_options = options;
}

SearchOptions SearchController::options() const
{
  return m_options;
}
