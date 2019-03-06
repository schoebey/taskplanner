#ifndef LINKWIDGET_H
#define LINKWIDGET_H

#include <QStyleOption>
#include <QFrame>
#include <QUrl>

namespace Ui {
  class LinkWidget;
}

enum CustomStyleOptions
{
  eLinkWidget = QStyleOption::SO_CustomBase + 1
};

enum CustomControlElements
{
  CE_LinkWidget = QStyle::CE_CustomBase + 1
};

class QStyleOptionLinkWidget : public QStyleOption
{
public:
  QStyleOptionLinkWidget();

  QRect labelRect;
  QRect iconRect;
  bool bDrawFrame = false;
  double dPaddingX = 0;
  double dPaddingY = 0;
  double dBorderRadius = 0;
};

class LinkWidget : public QFrame
{
  Q_OBJECT
public:
  LinkWidget(const QUrl& link);
  ~LinkWidget();

  Q_PROPERTY(bool constrainLabelToSize READ constrainLabelToSize WRITE setConstrainLabelToSize)
  void setConstrainLabelToSize(bool);
  bool constrainLabelToSize() const;

  Q_PROPERTY(bool drawFrame READ drawFrame WRITE setDrawFrame)
  void setDrawFrame(bool);
  bool drawFrame() const;

  Q_PROPERTY(double borderRadius READ borderRadius WRITE setBorderRadius)
  void setBorderRadius(double dRadius);
  double borderRadius() const;

private:
  void enterEvent(QEvent* pEvent) override;
  void leaveEvent(QEvent* pEvent) override;
  void paintEvent(QPaintEvent*) override;

protected:
  Ui::LinkWidget* ui;
  QUrl m_link;
  bool m_bConstrainLabelToSize = false;
  bool m_bDrawFrame = false;
  double m_dBorderRadius = 0;
  LinkWidget* m_pToolTip = nullptr;
};

#endif // LINKWIDGET_H
