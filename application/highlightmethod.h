#ifndef HIGHLIGHTMETHOD_H
#define HIGHLIGHTMETHOD_H

#include <QFlags>

enum EHighlightMethod
{
  eNoHighlight = 0x0,
  eFocus = 0x1,
  eValueAccepted = 0x2, // non-permanent flag
  eValueRejected = 0x4, // non-permanent flag
  eTimeTrackingActive = 0x8,
  eHover = 0x10,
  eInsertPossible = 0x20,
  eSearchResult = 0x40,
  eActiveSearchResult = 0x41
};
Q_DECLARE_FLAGS(HighlightingMethod, EHighlightMethod)

#endif // HIGHLIGHTMETHOD_H
