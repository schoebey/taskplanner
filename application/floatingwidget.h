#ifndef FLOATINGWIDGET_H
#define FLOATINGWIDGET_H

#include <QFrame>

class FloatingWidget : public QFrame
{
  Q_OBJECT
public:
  FloatingWidget(QWidget* pParent);
private:
  bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // FLOATINGWIDGET_H
