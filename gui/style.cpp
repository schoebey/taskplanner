#include "style.h"

#include "editablelabel.h"
#include "linkwidget.h"

#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QTextLayout>

#include <cmath>

typedef void(*tfnDrawText)(QPainter*, const QPointF&, const QString&,
                           const QColor&, const QColor&);

namespace
{
void drawShadowedText(QPainter* pPainter, const QPointF& pt, const QString& sText,
                      const QColor& col, const QColor& shadowColor)
{
  if (nullptr == pPainter)  { return; }

  pPainter->save();
  QFont f(pPainter->font());
  QFontMetrics m(f);

  QString sKey = QString("%1_%2x%3_%4_outline").arg(sText).arg(m.width(sText)).arg(m.height()).arg(shadowColor.name());

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
      p.setPen(Qt::NoPen);
      p.setBrush(col);
      p.drawPath(textPath);
      QPixmapCache::insert(sKey, QPixmap::fromImage(img));
      pPixmap = QPixmapCache::find(sKey);
    }
  }

  pPainter->restore();

  if (nullptr != pPixmap)
  {
    pPainter->drawPixmap(pt.x(), pt.y(), *pPixmap);
  }
}

void drawOutlinedText(QPainter* pPainter, const QPointF& pt, const QString& sText,
                      const QColor& col, const QColor& shadowColor)
{
  if (nullptr == pPainter)  { return; }

  pPainter->save();

  QFont f(pPainter->font());
  QFontMetrics m(f);

  QString sKey = QString("%1_%2x%3_%4_shadow").arg(sText).arg(m.width(sText)).arg(m.height()).arg(shadowColor.name());

  QPixmap* pPixmap = QPixmapCache::find(sKey);
  if (nullptr == pPixmap)
  {
    QImage img(m.width(sText), m.height() + 2, QImage::Format_ARGB32);
    if (!img.isNull())
    {
      img.fill(Qt::transparent);

      QPainterPath textPath;
      textPath.addText(0, img.height()-2, f, sText);

      QPainter p(&img);
      p.setPen(QPen(shadowColor, 5));
      p.setBrush(shadowColor);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::HighQualityAntialiasing, true);
      p.setRenderHint(QPainter::SmoothPixmapTransform, true);
      p.drawPath(textPath);

      p.setCompositionMode(QPainter::CompositionMode_SourceOver);
      p.setPen(Qt::NoPen);
      p.setBrush(col);
      p.drawPath(textPath);
      QPixmapCache::insert(sKey, QPixmap::fromImage(img));
      pPixmap = QPixmapCache::find(sKey);
    }
  }

  pPainter->restore();

  if (nullptr != pPixmap)
  {
    pPainter->drawPixmap(pt.x(), pt.y(), *pPixmap);
  }
}


  void qt_format_text(tfnDrawText fnDrawText,
                      const QFont& font,
                      const QRectF &_r,
                      int tf,
                      const QTextOption *option,
                      const QString& str,
                      QRectF *brect,
                      QPainter *painter = nullptr,
                      const QColor& textColor = QColor(),
                      const QColor& shadowColor = QColor())
  {
      Q_ASSERT( !((tf & ~Qt::TextDontPrint)!=0 && option!=0) ); // we either have an option or flags
      if (option) {
          tf |= option->alignment();
          if (option->wrapMode() != QTextOption::NoWrap)
              tf |= Qt::TextWordWrap;
          if (option->flags() & QTextOption::IncludeTrailingSpaces)
              tf |= Qt::TextIncludeTrailingSpaces;
      }
      // we need to copy r here to protect against the case (&r == brect).
      QRectF r(_r);
      bool dontclip  = (tf & Qt::TextDontClip);
      bool wordwrap  = (tf & Qt::TextWordWrap) || (tf & Qt::TextWrapAnywhere);
      bool singleline = (tf & Qt::TextSingleLine);
      bool showmnemonic = (tf & Qt::TextShowMnemonic);
      bool hidemnmemonic = (tf & Qt::TextHideMnemonic);
      Qt::LayoutDirection layout_direction;
      if (tf & Qt::TextForceLeftToRight)
          layout_direction = Qt::LeftToRight;
      else if (tf & Qt::TextForceRightToLeft)
          layout_direction = Qt::RightToLeft;
      else if (option)
          layout_direction = option->textDirection();
      else if (painter)
          layout_direction = painter->layoutDirection();
      else
          layout_direction = Qt::LeftToRight;
      bool isRightToLeft = layout_direction == Qt::RightToLeft;
      bool expandtabs = ((tf & Qt::TextExpandTabs) &&
                          (((tf & Qt::AlignLeft) && !isRightToLeft) ||
                            ((tf & Qt::AlignRight) && isRightToLeft)));
      if (!painter)
          tf |= Qt::TextDontPrint;
      uint maxUnderlines = 0;

      QString text = str;
      int offset = 0;
  start_lengthVariant:
      bool hasMoreLengthVariants = false;
      // compatible behaviour to the old implementation. Replace
      // tabs by spaces
      int old_offset = offset;
      for (; offset < text.length(); offset++) {
          QChar chr = text.at(offset);
          if (chr == QLatin1Char('\r') || (singleline && chr == QLatin1Char('\n'))) {
              text[offset] = QLatin1Char(' ');
          } else if (chr == QLatin1Char('\n')) {
              text[offset] = QChar::LineSeparator;
          } else if (chr == QLatin1Char('&')) {
              ++maxUnderlines;
          } else if (chr == QLatin1Char('\t')) {
              if (!expandtabs) {
                  text[offset] = QLatin1Char(' ');
              }
          } else if (chr == QChar(ushort(0x9c))) {
              // string with multiple length variants
              hasMoreLengthVariants = true;
              break;
          }
      }
      QVector<QTextLayout::FormatRange> underlineFormats;
      int length = offset - old_offset;
      if ((hidemnmemonic || showmnemonic) && maxUnderlines > 0) {
          QChar *cout = text.data() + old_offset;
          QChar *cout0 = cout;
          QChar *cin = cout;
          int l = length;
          while (l) {
              if (*cin == QLatin1Char('&')) {
                  ++cin;
                  --length;
                  --l;
                  if (!l)
                      break;
                  if (*cin != QLatin1Char('&') && !hidemnmemonic && !(tf & Qt::TextDontPrint)) {
                      QTextLayout::FormatRange range;
                      range.start = cout - cout0;
                      range.length = 1;
                      range.format.setFontUnderline(true);
                      underlineFormats.append(range);
                  }
  #ifdef Q_OS_MAC
              } else if (hidemnmemonic && *cin == QLatin1Char('(') && l >= 4 &&
                         cin[1] == QLatin1Char('&') && cin[2] != QLatin1Char('&') &&
                         cin[3] == QLatin1Char(')')) {
                  int n = 0;
                  while ((cout - n) > cout0 && (cout - n - 1)->isSpace())
                      ++n;
                  cout -= n;
                  cin += 4;
                  length -= n + 4;
                  l -= 4;
                  continue;
  #endif //Q_OS_MAC
              }
              *cout = *cin;
              ++cout;
              ++cin;
              --l;
          }
      }
      qreal height = 0;
      qreal width = 0;
      QString finalText = text.mid(old_offset, length);


      QFontMetricsF fm(font);
      QTextLayout textLayout(finalText, font);
      textLayout.setCacheEnabled(true);
      if (finalText.isEmpty()) {
          height = fm.height();
          width = 0;
          tf |= Qt::TextDontPrint;
      } else {
          qreal lineWidth = 0x01000000;
          if (wordwrap || (tf & Qt::TextJustificationForced))
              lineWidth = qMax<qreal>(0, r.width());
          if(!wordwrap)
              tf |= Qt::TextIncludeTrailingSpaces;
          textLayout.beginLayout();
          qreal leading = fm.leading();
          height = -leading;
          while (1) {
              QTextLine l = textLayout.createLine();
              if (!l.isValid())
                  break;
              l.setLineWidth(lineWidth);
              height += leading;
              // Make sure lines are positioned on whole pixels
              height = ceil(height);
              l.setPosition(QPointF(0., height));
              height += l.height();
              QString s = finalText.mid(l.textStart(), l.textLength());
              width = qMax(width, l.naturalTextWidth());
              if (!dontclip && !brect && height >= r.height())
                  break;
          }
          textLayout.endLayout();
      }
      qreal yoff = -2;
      qreal xoff = 0;
      if (tf & Qt::AlignBottom)
          yoff = r.height() - height;
      else if (tf & Qt::AlignVCenter)
          yoff = (r.height() - height)/2 - 2;
      if (tf & Qt::AlignRight)
          xoff = r.width() - width;
      else if (tf & Qt::AlignHCenter)
          xoff = (r.width() - width)/2;
      QRectF bounds = QRectF(r.x() + xoff, r.y() + yoff, width, height);
      if (hasMoreLengthVariants && !(tf & Qt::TextLongestVariant) && !r.contains(bounds)) {
          offset++;
          goto start_lengthVariant;
      }
      if (brect)
          *brect = bounds;
      if (!(tf & Qt::TextDontPrint)) {
          bool restore = false;
          if (!dontclip && !r.contains(bounds)) {
              restore = true;
              painter->save();
              painter->setClipRect(r, Qt::IntersectClip);
          }
          for (int i = 0; i < textLayout.lineCount(); i++) {
              QTextLine line = textLayout.lineAt(i);

              xoff = 0;
              if (tf & Qt::AlignRight) {
                xoff = r.width() - width;
              }
              else if (tf & Qt::AlignHCenter)
                xoff = (r.width() - width) / 2;

              //line.draw(painter, QPointF(r.x() + xoff, r.y() + yoff));
              fnDrawText(painter, QPointF(line.x() + r.x() + xoff, line.y() + r.y() + yoff),
                         finalText.mid(line.textStart(),
                                       line.textLength()),
                          textColor, shadowColor);
          }
//          painter->setPen(Qt::yellow);
//          painter->drawRect(bounds);
          if (restore) {
              painter->restore();
          }
      }
  }

}

Style::Style()
{
  setBaseStyle(nullptr);
}

void Style::drawControl(ControlElement element,
                        const QStyleOption* pOption,
                        QPainter* pPainter,
                        const QWidget* pWidget) const
{
  switch (element)
  {
  case CustomControlElements::CE_LinkWidget:
  {
    const QStyleOptionLinkWidget* pLinkOption = qstyleoption_cast<const QStyleOptionLinkWidget*>(pOption);
    if (nullptr != pLinkOption)
    {
      if (pLinkOption->bDrawFrame)
      {
        static const double c_dBias = 0.5;
        QRectF r0 = pLinkOption->iconRect;
        r0.adjust(-pLinkOption->dPaddingX + c_dBias, -pLinkOption->dPaddingY + c_dBias,
                  pLinkOption->dPaddingX - c_dBias, pLinkOption->dPaddingY - c_dBias);

        QRectF r1 = pLinkOption->labelRect;
        r1.adjust(-pLinkOption->dPaddingX + c_dBias, -pLinkOption->dPaddingY + c_dBias,
                  pLinkOption->dPaddingX - c_dBias, pLinkOption->dPaddingY - c_dBias);

        QPolygonF p;
        p << r0.topLeft()
          << r0.topRight()
          << QPointF(r0.right(), r1.top())
          << r1.topRight()
          << r1.bottomRight()
          << r1.bottomLeft()
          << r1.topLeft()
          << QPointF(r0.left(), r1.top())
          << r0.topLeft()
          << r0.topRight();

        QPainterPath path;

        bool bStart = true;
        for (int i = 1; i < p.size(); ++i)
        {
          QVector2D v = QVector2D(p[i].x() - p[i-1].x(), p[i].y() - p[i-1].y());
          v = pLinkOption->dBorderRadius * v.normalized();
          if (bStart)
          {
            path.moveTo(p[1] - v.toPointF());
            bStart = false;
          }
          else
          {
            path.quadTo(p[i-1], p[i-1] + v.toPointF());
            path.lineTo(p[i] - v.toPointF());
          }
        }

        pPainter->save();
        pPainter->setRenderHint(QPainter::Antialiasing, true);
        pPainter->setPen(pOption->palette.color(QPalette::Text));
        pPainter->setBrush(pOption->palette.color(QPalette::AlternateBase));
        pPainter->drawPath(path);
        pPainter->restore();
      }
    }
  } break;
  default:
    return QProxyStyle::drawControl(element, pOption, pPainter, pWidget);
  }

}

void Style::drawItemText(QPainter* painter, const QRect& rect, int flags,
                         const QPalette& pal, bool /*enabled*/, const QString& text,
                         QPalette::ColorRole textRole) const
{
  QColor textColor = pal.color(textRole);
  QColor shadowColor(0, 0, 0, 100);


  EditableLabel* pLabel = dynamic_cast<EditableLabel*>(painter->device());
  bool bDrawOutline = false;
  if (nullptr != pLabel)
  {
    bDrawOutline = pLabel->drawOutline();
    if (bDrawOutline)
    {
      QColor outlineColor(0, 0, 0, 25);
      if (textColor.lightness() < 128)
      {
        outlineColor = QColor(255, 255, 255, 25);
      }
      shadowColor = outlineColor;
      shadowColor.setAlpha(100);

      qt_format_text(drawOutlinedText, painter->font(), rect, flags,
                     nullptr, text, nullptr, painter,
                     textColor, outlineColor);
    }
  }

  qt_format_text(drawShadowedText, painter->font(), rect, flags,
                 nullptr, text, nullptr, painter,
                 textColor, shadowColor);
}

QRect Style::itemTextRect(const QFontMetrics & metrics,
                          const QRect & rectangle,
                          int alignment,
                          bool /*enabled*/,
                          const QString & text) const
{

  return metrics.boundingRect(rectangle, alignment, text);

  QRectF rect;
  //qt_format_text(metrics, rectangle, alignment, nullptr, text, &rect);

  return rect.toRect();
}
