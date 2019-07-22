#ifndef TASKWIDGETOVERLAY_H
#define TASKWIDGETOVERLAY_H

#include "highlightmethod.h"

#include <QFrame>
#include <QVariant>

class TaskWidget;
class TaskWidgetOverlay : public QFrame
{
  Q_OBJECT
public:
  explicit TaskWidgetOverlay(QWidget* pParent = nullptr);

  void setHighlight(HighlightingMethod method);
  HighlightingMethod highlight() const;

  Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor)
  QColor highlightColor() const;
  void setHighlightColor(const QColor& color);
  void setHighlightColor(const QColor& color, int iMsecs);

  Q_PROPERTY(QBrush background READ background WRITE setBackground)
  QBrush background() const;
  void setBackground(const QBrush& b);

  Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
  QColor borderColor() const;
  void setBorderColor(const QColor& color);
  void setBorderColor(const QColor& color, int iMsecs);

signals:

public slots:

private:
  void paintEvent(QPaintEvent* pEvent) override;
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  HighlightingMethod m_method = EHighlightMethod::eNoHighlight;
  QColor m_borderColor = QColor(0,0,0,0);
  QColor m_highlightColor = QColor(0,0,0,0);
  QBrush m_backgroundBrush;
};

#endif // TASKWIDGETOVERLAY_H
