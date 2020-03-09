#ifndef LAYOUTUTILS_H
#define LAYOUTUTILS_H

class QWidget;
class QPoint;
class QGridLayout;
class FlowLayout;

namespace tools
{
  bool addToGridLayout(QWidget* pT, QGridLayout* pLayout, QPoint pt);

  bool addToFlowLayout(QWidget* pT, FlowLayout* pLayout, QPoint pt);
}

#endif // LAYOUTUTILS_H
