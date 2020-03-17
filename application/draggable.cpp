#include "draggable.h"


namespace detail
{
  void sendEnterMoveLeaveEvents(QMouseEvent* pMouseEvent,
                                QWidget*& pPreviouslyEntered)
  {
    // TODO: maybe emulate event dispatching since Qt won't do it?
    // e.g. dispatch enter/move/leave events
    // in order to do that, the origin of mouse events has to be analyzed...
    /* upon entering a new widget, store the previous recipient of the enter event
     * when entering a new widget, check the previously entered widget:
     * is the mouse pos still within the widget? do nothing
     * is the mouse pos outside the widget? send leave event to every ancestor of the widget that has previously received the enter event
     * do this by checking the widget's flag wa_undermouse or pWidget->underMouse();
     * note: might not work since qt doc mentions the flag not being updated during drag&drop operation,
     * which might include normal operation when mouse button is down...
     *
     *
     */
    QPoint pt = pMouseEvent->globalPos();
    QWidget* pCurr = qApp->widgetAt(pMouseEvent->globalPos());
    if (pPreviouslyEntered != pCurr)
    {
      QWidget* pPrev = pPreviouslyEntered;
      QPoint mappedPt = pPrev->mapFromGlobal(pt);
      while (nullptr != pPrev &&
             !pPrev->rect().contains(mappedPt))
      {
        QEvent e(QEvent::Leave);
        QCoreApplication::sendEvent(pPrev, &e);
        pPrev = pPrev->parentWidget();
        mappedPt = pPrev->mapFromGlobal(pt);
      }

      pPreviouslyEntered = pCurr;

      // TODO: from the current widget towards its parent until pPrev (the first parent that still contained the point),
      // send enterevents to each of them
      QWidget* pAlreadyReceivedEnterEvent = pPrev;
      pPrev = pCurr;
      mappedPt = pPrev->mapFromGlobal(pt);
      while (nullptr != pPrev &&
             pPrev != pAlreadyReceivedEnterEvent &&
             pPrev->rect().contains(mappedPt))
      {
        QEvent e(QEvent::Enter);
        QCoreApplication::sendEvent(pPrev, &e);
        pPrev = pPrev->parentWidget();
        mappedPt = pPrev->mapFromGlobal(pt);
      }
    }
    else
    {
      QPoint mappedPt = pCurr->mapFromGlobal(pt);
      QMouseEvent e(QEvent::Move, mappedPt, pMouseEvent->button(), pMouseEvent->buttons(), pMouseEvent->modifiers());
      QCoreApplication::sendEvent(pCurr, &e);
    }
  }
}
