#ifndef HIGHLIGHTMETHOD_H
#define HIGHLIGHTMETHOD_H

#include <QFlags>

enum EHighlightMethod
{
  eNoHighlight = 0,
  eFocus = 1,
  eValueAccepted = 2,
  eValueRejected = 4,
  eTimeTrackingActive = 8,
};
Q_DECLARE_FLAGS(HighlightingMethod, EHighlightMethod)

#endif // HIGHLIGHTMETHOD_H
