#ifndef COLORSPACEWIDGET_H
#define COLORSPACEWIDGET_H

#include <QWidget>
#include <QImage>

class ColorSpaceWidget : public QWidget
{
  Q_OBJECT

public:
  ColorSpaceWidget();

  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
  QColor color() const;
  void setColor(const QColor& c);

  Q_PROPERTY(double saturation READ saturation WRITE setSaturation NOTIFY saturationChanged)
  double saturation() const;
  void setSaturation(double dSaturation);

  void paintEvent(QPaintEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;
  void mousePressEvent(QMouseEvent* pEvent) override;
  void mouseMoveEvent(QMouseEvent* pEvent) override;

signals:
  void colorChanged(const QColor&);
  void saturationChanged(double);

private:
  QImage m_buffer;
  QColor m_currentCol;
  double m_dSaturation = 1;
};

#endif // COLORSPACEWIDGET_H
