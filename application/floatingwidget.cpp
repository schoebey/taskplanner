#include "floatingwidget.h"

#include <QEvent>
#include <QResizeEvent>

FloatingWidget::FloatingWidget(QWidget* pParent)
  : QFrame(pParent)
{
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setFocusPolicy(Qt::NoFocus);
  if (nullptr != pParent)
  {
    pParent->installEventFilter(this);
  }
}

bool FloatingWidget::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == parentWidget())
  {
    switch (event->type())
    {
    case QEvent::Resize:
    {
      QResizeEvent* pResizeEvent = dynamic_cast<QResizeEvent*>(event);
      if (nullptr != pResizeEvent)
      {
        move(parentWidget()->contentsRect().topLeft());
        auto margins = parentWidget()->contentsMargins();
        resize(pResizeEvent->size() - QSize(margins.left() + margins.right(),
                                            margins.top() + margins.bottom()));
      }
     } break;
    default:
      break;
    }
  }

  return false;
}
