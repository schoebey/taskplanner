#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "matchinfo.h"

#include <QObject>
#include <QString>
#include <QRect>

class Manager;
class WidgetManager;
class TaskWidget;

class SearchController : public QObject
{
  Q_OBJECT

public:
  typedef tvMatchInfo tMatches;
  SearchController(Manager* pManager,
                   WidgetManager* pWidgetManager);
  ~SearchController() override;

  //TaskWidget* currentTaskWidget() const;
  size_t hitCount() const;

  void onSearchTermChanged(const QString&);

  void onNext();

  void onPrev();

signals:
  void positionChanged(size_t currentPos, size_t totalHitCount);

private:
  void setCurrent(tMatches::iterator it);

private:
  Manager* m_pTaskManager = nullptr;
  WidgetManager* m_pWidgetManager = nullptr;
  tMatches m_hits;
  tMatches::iterator m_hitIter;
};

#endif // SEARCHCONTROLLER_H
