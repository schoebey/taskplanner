#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "overlaywidget.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public OverlayWidget
{
  Q_OBJECT

public:
  explicit AboutDialog(QWidget *parent = nullptr);
  ~AboutDialog();

  void setStatistics(const QString& sText);
private slots:
  void onLinkClicked(const QString& sLink);
private:
  Ui::AboutDialog *ui;
  QWidget* m_pWidget;
};

#endif // ABOUTDIALOG_H
