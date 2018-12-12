#ifndef TASKWIDGETOVERLAY_H
#define TASKWIDGETOVERLAY_H

#include "highlightmethod.h"

#include <QWidget>

class TaskWidget;
class TaskWidgetOverlay : public QWidget
{
  Q_OBJECT
public:
  explicit TaskWidgetOverlay(TaskWidget *parent = nullptr);

  void setHighlight(HighlightingMethod method);
  HighlightingMethod highlight() const;

  Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor)
  QColor highlightColor() const;
  void setHighlightColor(const QColor& color);
  void setHighlightColor(const QColor& color, int iMsecs);

  Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
  QColor borderColor() const;
  void setBorderColor(const QColor& color);
  void setBorderColor(const QColor& color, int iMsecs);

signals:

public slots:

private:
  void paintEvent(QPaintEvent* pEvent) override;

private:
  TaskWidget* m_pParent;
  HighlightingMethod m_method = EHighlightMethod::eNoHighlight;
  QColor m_borderColor = QColor(0,0,0,0);
  QColor m_highlightColor = QColor(0,0,0,0);
};

#endif // TASKWIDGETOVERLAY_H
