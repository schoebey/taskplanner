#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include <QLabel>

class TagWidget : public QLabel
{
public:
  TagWidget(const QString& sText, QWidget* pParent);
private:
  void paintEvent(QPaintEvent* pEvent) override;
};

#endif // TAGWIDGET_H
