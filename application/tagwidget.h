#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include "id_types.h"

#include <QLabel>
#include <QFrame>
#include <chrono>

#include <cmath>
#include <cassert>
#include <QStyleOption>


class EditableLabel;
class QStyleOptionTagWidget : public QStyleOption
{
public:
  QStyleOptionTagWidget();

  QColor color;
};

class TagWidget : public QFrame
{
  Q_OBJECT

public:
  TagWidget(tag_id id, const QString& sText, QWidget* pParent);

  TagWidget(const TagWidget& other);

  ~TagWidget();

  tag_id id() const;

  Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
  void setText(const QString& sText);
  QString text() const;

  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
  void setColor(const QColor& c);
  QColor color() const;

  Q_PROPERTY(double angle READ angle WRITE setAngle NOTIFY angleChanged)
  void setAngle(double dAngle);
  double angle() const;

  Q_PROPERTY(QPoint origin READ origin WRITE setOrigin NOTIFY originChanged)
  void setOrigin(QPoint pt);
  QPoint origin() const;

  Q_PROPERTY(bool editable READ editable WRITE setEditable NOTIFY editabilityChanged)
  void setEditable(bool bEditable);
  bool editable() const;

signals:
  void textChanged(const QString&);
  void colorChanged(const QColor&);
  void angleChanged(double);
  void originChanged(const QPoint&);
  void editabilityChanged();

protected:
  bool event(QEvent* pEvent);
  void paintEvent(QPaintEvent* pEvent) override;
  void mousePressEvent(QMouseEvent* pEvent) override;
  void mouseMoveEvent(QMouseEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;
  void timerEvent(QTimerEvent* event) override;
  QSize sizeHint() const override;

private:
  void startSimulation();
  void stopSimulation();
  void stepSimulation();

private:
  tag_id m_id;
  EditableLabel* m_pLabel;
  QColor m_color;
  double m_dAngleRad = 0;
  QPoint m_origin;
  QSize m_size;
  QSize m_expandedSize;
  int m_iTimerId = -1;
  QImage m_image;
  std::chrono::steady_clock::time_point m_startTime;
  double m_dSimulationStartAngleRad = 0;
  QAction* m_pColorPickerAction = nullptr;
};

#endif // TAGWIDGET_H
