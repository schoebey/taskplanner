#ifndef TASKCREATIONDIALOG_H
#define TASKCREATIONDIALOG_H

#include "overlaywidget.h"

class TaskWidget;
class TaskCreationDialog : public OverlayWidget
{
  Q_OBJECT
public:
  explicit TaskCreationDialog(QWidget *parent = nullptr);

  void show();

  QString name() const;
  QString description() const;

  void keyPressEvent(QKeyEvent* pEvent) override;
signals:
  void accepted();
  void rejected();

private slots:
  void accept();
  void reject();

private:
  TaskWidget* m_pTaskWidget = nullptr;
};

#endif // TASKCREATIONDIALOG_H
