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
};

#endif // STYLE_H
