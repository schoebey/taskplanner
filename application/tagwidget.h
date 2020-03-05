#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include "draggable.h"

#include <QLabel>
#include <QFrame>

#include <cmath>
#include <cassert>
#include <QStyleOption>


class QStyleOptionTagWidget : public QStyleOption
{
public:
  QStyleOptionTagWidget();

  QString sText;
  QColor color;
};

class TagWidget : public QLabel
{
public:
  TagWidget(const QString& sText, QWidget* pParent);

private:
  void paintEvent(QPaintEvent* pEvent) override;
  QSize sizeHint() const override;
};

using DraggableTagWidget = Draggable<TagWidget>;

#endif // TAGWIDGET_H
