#ifndef TAGWIDGETCONTAINER_H
#define TAGWIDGETCONTAINER_H

#include "tagwidget.h"
#include "draggable.h"

#include <QFrame>

class TagWidgetContainer : public QFrame, public DraggableContainer<TagWidget>
{
public:
  TagWidgetContainer(QWidget* pParent);
  ~TagWidgetContainer() = default;

private:
  bool addItem_impl(TagWidget* pT) override;
  bool removeItem_impl(TagWidget* pT) override;
  bool insertItem_impl(TagWidget* pT, QPoint pt) override;
};

#endif // TAGWIDGETCONTAINER_H
