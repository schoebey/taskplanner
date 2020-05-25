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

  Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
  void setText(const QString& sText);
  QString text() const;

  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
  void setColor(const QColor& c);
  QColor color() const;

signals:
  void textChanged(const QString&);
  void colorChanged(const QColor&);
private:
  void paintEvent(QPaintEvent* pEvent) override;
  void showEvent(QShowEvent* pEvent);
  void hideEvent(QHideEvent* pEvent);
  QSize sizeHint() const override;

private:
  QString m_sText;
  QColor m_color;
};

using DraggableTagWidget = Draggable<TagWidget>;

#endif // TAGWIDGET_H
