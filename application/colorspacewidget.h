#ifndef COLORSPACEWIDGET_H
#define COLORSPACEWIDGET_H

#include <QWidget>
#include <QImage>

class ColorSpaceWidget : public QWidget
{
  Q_OBJECT

public:
  ColorSpaceWidget(QWidget* pParent = nullptr);

  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
  QColor color() const;
  void setColor(const QColor& c);

  Q_PROPERTY(double saturation READ saturation WRITE setSaturation NOTIFY saturationChanged)
  unsigned char saturation() const;
  void setSaturation(int sat);

  void paintEvent(QPaintEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;
  void mousePressEvent(QMouseEvent* pEvent) override;
  void mouseMoveEvent(QMouseEvent* pEvent) override;

signals:
  void colorChanged(const QColor&);
  void saturationChanged(int);

private:
  void reCreateBuffer(QSize s);

private:
  QImage m_buffer;
  QColor m_currentCol;
  int m_saturation = 255;
};

#endif // COLORSPACEWIDGET_H
