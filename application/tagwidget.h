#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include "draggable.h"

#include <QLabel>
#include <QFrame>

#include <cmath>
#include <cassert>


class TagWidget : public QLabel, public Draggable<TagWidget>
{
public:
  TagWidget(const QString& sText, QWidget* pParent);
private:
  void paintEvent(QPaintEvent* pEvent) override;
};


#endif // TAGWIDGET_H
