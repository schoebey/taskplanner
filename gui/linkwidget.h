#ifndef LINKWIDGET_H
#define LINKWIDGET_H

#include <QWidget>
#include <QUrl>

class LinkWidget : public QWidget
{
  Q_OBJECT
public:
  LinkWidget(const QUrl& link);
  ~LinkWidget();

private:
  void paintEvent(QPaintEvent*);

private:
  QUrl m_link;
};

#endif // LINKWIDGET_H
