#ifndef LINKWIDGET_H
#define LINKWIDGET_H

#include <QFrame>
#include <QUrl>

namespace Ui {
  class LinkWidget;
}

class LinkWidget : public QFrame
{
  Q_OBJECT
public:
  LinkWidget(const QUrl& link);
  ~LinkWidget();

  Q_PROPERTY(bool constrainLabelToSize READ constrainLabelToSize WRITE setConstrainLabelToSize)
  void setConstrainLabelToSize(bool);
  bool constrainLabelToSize() const;

  void showToolTip();

private:
  void enterEvent(QEvent* pEvent) override;
  void leaveEvent(QEvent* pEvent) override;
  void paintEvent(QPaintEvent*) override;

protected:
  Ui::LinkWidget* ui;
  QUrl m_link;
  bool m_bConstrainLabelToSize = false;
  LinkWidget* m_pToolTip = nullptr;
};

#endif // LINKWIDGET_H
