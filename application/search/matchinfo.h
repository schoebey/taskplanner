#ifndef MATCHINFO_H
#define MATCHINFO_H

#include "id_types.h"

#include <QMetaType>
#include <QRect>
#include <QPointer>

#include <vector>
#include <deque>

class TaskWidget;
struct SMatchInfo
{
  QPointer<QWidget> pWidget;
  QRect rect;
  int iStart = -1;
  int iSize = -1;
  QPointer<TaskWidget> pClosestParentTaskWidget;
  std::deque<task_id> vToExpand;
};
typedef std::vector<SMatchInfo> tvMatchInfo;
Q_DECLARE_METATYPE(SMatchInfo)

#endif // MATCHINFO_H
