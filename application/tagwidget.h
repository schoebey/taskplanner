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

class TagWidget : public QWidget
{
  Q_OBJECT

public:
  TagWidget(const QString& sText, QWidget* pParent);

  TagWidget(const TagWidget& other);

  ~TagWidget();

  void setText(const QString& sText);
  QString text() const;

  void showEvent(QShowEvent* pEvent);
  void hideEvent(QHideEvent* pEvent);
private:
  void paintEvent(QPaintEvent* pEvent) override;
  QSize sizeHint() const override;

private:
  QString m_sText;
};

using DraggableTagWidget = Draggable<TagWidget>;

#endif // TAGWIDGET_H
