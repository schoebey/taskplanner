#include "style.h"

#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>

namespace
{
  void drawShadowedText(QPainter* pPainter, const QPoint& pt, const QString& sText,
                        const QColor& col, const QColor& shadowColor)
  {
    if (nullptr == pPainter)  { return; }

    QFont f(pPainter->font());
    QFontMetrics m(f);

    QString sKey = QString("%1_%2x%3_%4").arg(sText).arg(m.width(sText)).arg(m.height()).arg(shadowColor.name());

    QPixmap* pPixmap = QPixmapCache::find(sKey);
    if (nullptr == pPixmap)
    {
      QImage img(m.width(sText), m.height() + 2, QImage::Format_ARGB32);
      if (!img.isNull())
      {
        img.fill(Qt::transparent);

        QPainterPath textPath;
        textPath.addText(0, img.height()-2, f, sText);

        QPainterPath shadowPath;
        shadowPath.addText(0, img.height()-1, f, sText);

        QPainter p(&img);
        p.setPen(Qt::NoPen);
        p.setBrush(shadowColor);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::HighQualityAntialiasing, true);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        p.drawPath(shadowPath);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.setBrush(QColor(0,0,0,255));
        p.drawPath(textPath);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.setBrush(col);
        p.drawPath(textPath);
        QPixmapCache::insert(sKey, QPixmap::fromImage(img));
        pPixmap = QPixmapCache::find(sKey);
      }
    }


    if (nullptr != pPixmap)
    {
      pPainter->drawPixmap(pt.x(), pt.y(), *pPixmap);
    }
  }
}

Style::Style()
{
  setBaseStyle(nullptr);
}

void Style::drawItemText(QPainter* painter, const QRect& rect, int flags,
                         const QPalette& pal, bool enabled, const QString& text,
                         QPalette::ColorRole textRole) const
{
  if (text.contains("\n"))
  {
   QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
  }
  else
  {
    QPoint pt(rect.topLeft());
    if (flags & Qt::AlignLeft)
    {
      pt.setX(rect.left());
    }
    else if (flags & Qt::AlignHCenter)
    {
      pt.setX(rect.center().x() - painter->fontMetrics().width(text) / 2);
    }
    else if (flags & Qt::AlignRight)
    {
      pt.setX(rect.right() - painter->fontMetrics().width(text));
    }

    int iHeight = painter->fontMetrics().height();
    if (flags & Qt::AlignTop)
    {
      pt.setY(rect.top() - 2);
    }
    else if (flags & Qt::AlignVCenter)
    {
      pt.setY(rect.center().y() - iHeight / 2.0 - 2);
    }
    else if (flags & Qt::AlignBottom)
    {
      pt.setY(rect.bottom() - iHeight);
    }

    drawShadowedText(painter, pt, text, pal.color(textRole), QColor(0,0,0,100));
  }
}
