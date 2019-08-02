#ifndef BLENDABLEIMAGEWIDGET_H
#define BLENDABLEIMAGEWIDGET_H

#include <QFrame>

class BlendableImageWidget : public QFrame
{
  Q_OBJECT
public:
  BlendableImageWidget(QWidget* pParent = nullptr);

  Q_PROPERTY(double backgroundImageBlendFactor READ backgroundImageBlendFactor WRITE setBackgroundImageBlendFactor)
  double backgroundImageBlendFactor() const;
  void setBackgroundImageBlendFactor(double dFactor);


  void setBackgroundImage(const QImage& image);

private:
  void paintEvent(QPaintEvent* /*pEvent*/) override;
  void resizeEvent(QResizeEvent* pEvent) override;

private:
  QPixmap m_cache;
  QImage m_backgroundImage[2];
  double m_dBackgroundImageBlendFactor = 0;
};

#endif // BLENDABLEIMAGEWIDGET_H
