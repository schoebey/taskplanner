#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include <QLabel>
#include <QFrame>
#include <chrono>

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

  Q_PROPERTY(double angle READ angle WRITE setAngle NOTIFY angleChanged)
  void setAngle(double dAngle);
  double angle() const;

  Q_PROPERTY(QPoint origin READ origin WRITE setOrigin NOTIFY originChanged)
  void setOrigin(QPoint pt);
  QPoint origin() const;

signals:
  void textChanged(const QString&);
  void colorChanged(const QColor&);
  void angleChanged(double);
  void originChanged(const QPoint&);

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
  QString m_sText;
  QColor m_color;
  double m_dAngleRad = 0;
  QPoint m_origin;
  QSize m_size;
  QSize m_expandedSize;
  int m_iTimerId = -1;
  QImage m_image;
  std::chrono::steady_clock::time_point m_startTime;
  double m_dSimulationStartAngleRad = 0;
};

#endif // TAGWIDGET_H
