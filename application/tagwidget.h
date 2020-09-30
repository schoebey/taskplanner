#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include "id_types.h"

#include <QLabel>
#include <QFrame>

#include <cmath>
#include <cassert>
#include <QStyleOption>


class EditableLabel;

class TagWidget : public QFrame
{
  Q_OBJECT

public:
  TagWidget(tag_id id, const QString& sText, QWidget* pParent);

  TagWidget(const TagWidget& other) = delete;

  ~TagWidget();

  tag_id id() const;

  Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
  void setText(const QString& sText);
  QString text() const;

  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
  void setColor(const QColor& c);
  QColor color() const;

  Q_PROPERTY(bool editable READ editable WRITE setEditable NOTIFY editabilityChanged)
  void setEditable(bool bEditable);
  bool editable() const;

  void paintEvent(QPaintEvent*);

signals:
  void textChanged(const QString&);
  void colorChanged(const QColor&);
  void editabilityChanged();

private:
  tag_id m_id;
  QString m_sText;
  QColor m_color;
  EditableLabel* m_pLabel = nullptr;
  QAction* m_pColorPickerAction = nullptr;
};

#endif // TAGWIDGET_H
