#ifndef TAGWIDGETCONTAINER_H
#define TAGWIDGETCONTAINER_H

#include "tagwidget.h"
#include "draggable.h"

#include <QFrame>

class TagWidgetContainer : public DraggableContainer<DraggableTagWidget>
{
public:
  TagWidgetContainer(QWidget* pParent);
  ~TagWidgetContainer() = default;

private:
  bool addItem_impl(DraggableTagWidget* pT) override;
  bool removeItem_impl(DraggableTagWidget* pT) override;
  bool insertItem_impl(DraggableTagWidget* pT, QPoint pt) override;
  bool showPlaceholderAt(const QPoint& pt, const QSize& s) override;
  void hidePlaceholder() override;

  QFrame* m_pPlaceholder;
};

#endif // TAGWIDGETCONTAINER_H
