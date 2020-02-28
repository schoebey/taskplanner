#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>

class ColorSpaceWidget;
class QSlider;

class ColorPicker : public QWidget
{
  Q_OBJECT
public:
  ColorPicker(QWidget* pParent = nullptr);

  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
  QColor color() const;
  void setColor(const QColor& c);

signals:
  void colorChanged(const QColor&);

private slots:
  void onSaturationChanged(int iSat);

private:
  ColorSpaceWidget* m_pCsw;
  QSlider* m_pSatSlider;
};

#endif // COLORPICKER_H
