#ifndef MATCHINFO_H
#define MATCHINFO_H

#include <QMetaType>
#include <QRect>

#include <vector>

struct SMatchInfo
{
  QWidget* pWidget = nullptr;
  QRect rect;
  int iStart = -1;
  int iSize = -1;
};
typedef std::vector<SMatchInfo> tvMatchInfo;
Q_DECLARE_METATYPE(SMatchInfo)

#endif // MATCHINFO_H
