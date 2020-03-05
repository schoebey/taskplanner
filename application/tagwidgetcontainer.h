#ifndef TAGWIDGETCONTAINER_H
#define TAGWIDGETCONTAINER_H

#include "tagwidget.h"
#include "draggable.h"

#include <QFrame>

class TagWidgetContainer : public QFrame, public DraggableContainer<DraggableTagWidget>
{
public:
  TagWidgetContainer(QWidget* pParent);
  ~TagWidgetContainer() = default;

private:
  bool addItem_impl(DraggableTagWidget* pT) override;
  bool removeItem_impl(DraggableTagWidget* pT) override;
  bool insertItem_impl(DraggableTagWidget* pT, QPoint pt) override;
};

#endif // TAGWIDGETCONTAINER_H
