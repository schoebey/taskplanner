#ifndef TAGWIDGETCONTAINER_H
#define TAGWIDGETCONTAINER_H

#include "tagwidget.h"
#include "draggable.h"

#include <QFrame>

using DraggableTagWidget = Draggable<TagWidget>;

class TagWidgetContainer : public DraggableContainer<DraggableTagWidget>
{
  Q_OBJECT
public:
  TagWidgetContainer(QWidget* pParent);
  ~TagWidgetContainer() = default;

  void setEditable(bool);

  bool modifyTag(tag_id id, const QString& sNewName, const QColor& col);

signals:
  void tagAdded(DraggableTagWidget*);
  void tagRemoved(DraggableTagWidget*);
  void tagMoved(DraggableTagWidget*, DraggableContainer<DraggableTagWidget>*);
  void tagChanged(tag_id tagId, const QString& sNewName, const QColor& col);
  void newTagRequested(const QString& sTagName, TagWidgetContainer* pContainer);

protected:
  void emitItemAdded(DraggableTagWidget*) override;
  void emitItemInserted(DraggableTagWidget* pT, QPoint) override;
  void emitItemRemoved(DraggableTagWidget*) override;
  void emitItemMovedFrom(DraggableTagWidget*, DraggableContainer<DraggableTagWidget>*) override;

private:
  bool addItem_impl(DraggableTagWidget* pT) override;
  bool removeItem_impl(DraggableTagWidget* pT) override;
  bool insertItem_impl(DraggableTagWidget* pT, QPoint pt) override;
  bool moveItemFrom_impl(DraggableContainer<DraggableTagWidget>* pSource, DraggableTagWidget* pT, QPoint pt) override;
  bool showPlaceholderAt_impl(const QPoint& pt, const QSize& s) override;
  void hidePlaceholder() override;

  QFrame* m_pPlaceholder;
  bool m_bEditable = false;
};

#endif // TAGWIDGETCONTAINER_H
