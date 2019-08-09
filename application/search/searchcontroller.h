#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include <QObject>
#include <QString>

class Manager;
class WidgetManager;
class TaskWidget;

class SearchController : public QObject
{
  Q_OBJECT

  typedef std::vector<TaskWidget*> tvHitType;

public:
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
  void setCurrent(tvHitType::iterator it);

private:
  Manager* m_pTaskManager = nullptr;
  WidgetManager* m_pWidgetManager = nullptr;
  tvHitType m_vpHits;
  tvHitType::iterator m_hitIter;
};

#endif // SEARCHCONTROLLER_H
