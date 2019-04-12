#ifndef LINKWIDGET_H
#define LINKWIDGET_H

#include <QStyleOption>
#include <QFrame>
#include <QUrl>
#include <QMenu>

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

class QNetworkAccessManager;
class QNetworkReply;
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

signals:
  void deleteTriggered(QUrl);

private:
  void enterEvent(QEvent* pEvent) override;
  void leaveEvent(QEvent* pEvent) override;
  void paintEvent(QPaintEvent*) override;
  void mousePressEvent(QMouseEvent*) override;
  void mouseReleaseEvent(QMouseEvent*) override;
  void mouseDoubleClickEvent(QMouseEvent*) override;
  void contextMenuEvent(QContextMenuEvent*) override;

  void showOverlay();

  void hideOverlay();

private slots:
  void onDeleteTriggered();
  void openLink();
  void fileDownloaded();
  void onReadyRead();

protected:
  Ui::LinkWidget* ui;
  QUrl m_link;
  bool m_bConstrainLabelToSize = false;
  bool m_bDrawFrame = false;
  double m_dBorderRadius = 0;
  LinkWidget* m_pToolTip = nullptr;
  QMenu* m_pContextMenu = nullptr;
  std::shared_ptr<QNetworkAccessManager> m_spNetworkAccessManager;
  QByteArray m_iconFromWeb;
};

#endif // LINKWIDGET_H
