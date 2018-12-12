#ifndef TASKWIDGETOVERLAY_H
#define TASKWIDGETOVERLAY_H

#include <QWidget>

class TaskWidget;
class TaskWidgetOverlay : public QWidget
{
  Q_OBJECT
public:
  explicit TaskWidgetOverlay(TaskWidget *parent = nullptr);

signals:

public slots:

private:
  void paintEvent(QPaintEvent* pEvent) override;

private:
  TaskWidget* m_pParent;
};

#endif // TASKWIDGETOVERLAY_H
