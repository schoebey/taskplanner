#ifndef STYLE_H
#define STYLE_H

#include <QProxyStyle>

class Style : public QProxyStyle
{
public:
  Style();

  void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal,
                    bool enabled, const QString &text,
                    QPalette::ColorRole textRole = QPalette::NoRole) const override;

  QRect itemTextRect(const QFontMetrics& metrics, const QRect& rectangle,
                     int alignment, bool enabled, const QString& text) const override;
  void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;

  int pixelMetric(PixelMetric metric, const QStyleOption *option = Q_NULLPTR, const QWidget *widget = Q_NULLPTR) const override;
};

#endif // STYLE_H
