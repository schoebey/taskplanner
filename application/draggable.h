#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include <QMouseEvent>
#include <QApplication>

#include <cmath>
#include <cassert>
#include <QWidget>

class EventFilterHandler
{
public:
  virtual bool onEventFilter(QObject* pObject, QEvent* pEvent) = 0;

protected:
  EventFilterHandler() = default;
  ~EventFilterHandler() = default;
};

class EventFilter : public QObject
{
  Q_OBJECT
public:
  EventFilter(EventFilterHandler* pHandler)
    : m_pHandler(pHandler)
  {}

  ~EventFilter() = default;

  bool eventFilter(QObject* pObject, QEvent* pEvent) override
  {
    return m_pHandler->onEventFilter(pObject, pEvent);
  }
private:
  EventFilterHandler* m_pHandler = nullptr;
};


template<typename T>
class DraggableContainer : public EventFilterHandler
{
public:
  DraggableContainer(QObject* pThis)
    : m_eventFilter(this),
      m_pThis(pThis)
  {}

  ~DraggableContainer()
  {}

  bool addItem(T* pT)
  {
    if (nullptr == pT) { return false; }
    if (addItem_impl(pT))
    {
      pT->setContainer(this);
      return true;
    }
    return false;
  }

  bool removeItem(T* pT)
  {
    if (nullptr == pT) { return false; }
    if (removeItem_impl(pT))
    {
      return true;
    }
    return false;
  }

  bool insertItemAt(T* pT, QPoint pt)
  {
    if (nullptr == pT) { return false; }
    if (insertItem_impl(pT, pt))
    {
      pT->setContainer(this);
      return true;
    }
    return false;
  }

  static DraggableContainer<T>* containerUnderMouse()
  {
    return m_pContainerUnderMouse;
  }

private:
  virtual bool addItem_impl(T* pT) = 0;
  virtual bool removeItem_impl(T* pT) = 0;
  virtual bool insertItem_impl(T* pT, QPoint pt) = 0;

  void onMouseMove(QMouseEvent* /*pMouseEvent*/)
  {
    m_pContainerUnderMouse = this;
  }

  bool onEventFilter(QObject* pObj, QEvent* pEvent) override
  {
    if (m_pThis == pObj)
    {
      switch (pEvent->type())
      {
      case QEvent::MouseMove:
        onMouseMove(dynamic_cast<QMouseEvent*>(pEvent));
        break;
      default:
        break;
      }
    }

    return false;
  }

  static DraggableContainer<T>* m_pContainerUnderMouse;
  EventFilter m_eventFilter;
  QObject* m_pThis;
};

template<typename T>
class Draggable : public EventFilterHandler
{
public:
  Draggable(T* pThis)
    : m_pThis(pThis),
      m_eventFilter(this)
  {
    qApp->installEventFilter(&m_eventFilter);
  }

  ~Draggable()
  {}

  void setMouseDown(bool bMouseDown, QPoint pt = QPoint())
  {
    m_bMouseDown = bMouseDown;
    m_mouseDownPoint = pt;
  }

  bool mouseDown() const
  {
    return m_bMouseDown;
  }

  QPoint mouseDownPoint() const
  {
    return m_mouseDownPoint;
  }

  void setDraggingInstance(T* pT)
  {
    m_pDraggingInstance = pT;
  }

  T* draggingInstance() const
  {
    return m_pDraggingInstance;
  }

  void setContainer(DraggableContainer<T>* pContainer)
  {
    m_pContainer = pContainer;
  }

  DraggableContainer<T>* container() const
  {
    return m_pContainer;
  }

  void onMouseMove(QMouseEvent* pMouseEvent)
  {
    if (mouseDown() && nullptr == draggingInstance())
    {
      QPoint ptDist = pMouseEvent->pos() - mouseDownPoint();
      if (20 < std::sqrt(ptDist.x() * ptDist.x() + ptDist.y() * ptDist.y()))
      {
        setDraggingInstance(m_pThis);

        if (nullptr != m_pContainer)
        {
          m_pContainer->removeItem(m_pThis);
        }

        m_pThis->setParent(m_pThis->window());
        m_pThis->setFocus();
        m_pThis->move(pMouseEvent->globalPos() - mouseDownPoint());
        m_pThis->show();
        m_pThis->raise();
        pMouseEvent->accept();
      }
    }
  }

  void onMousePress(QMouseEvent* pMouseEvent)
  {
    setMouseDown(true, pMouseEvent->pos());
  }

  void onMouseRelease(QMouseEvent* /*pMouseEvent*/)
  {
    setMouseDown(false);
  }

  bool onEventFilter(QObject* pObj, QEvent* pEvent) override
  {
    if (this == m_pDraggingInstance)
    {
      if (QEvent::MouseMove == pEvent->type())
      {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
        m_pThis->move(m_pThis->parentWidget()->mapFromGlobal(pMouseEvent->globalPos() - m_mouseDownPoint));
      }
      else if (QEvent::MouseButtonRelease == pEvent->type())
      {
        m_bMouseDown = false;
        if (nullptr != DraggableContainer<T>::containerUnderMouse())
        {
          QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
          QWidget* pWidget = dynamic_cast<QWidget*>(this);
          QPoint pt = pMouseEvent->globalPos();
          if (nullptr != pWidget) {
            pt = pWidget->mapFromGlobal(pt);
          }
          DraggableContainer<T>::containerUnderMouse()->insertItemAt(m_pThis, pt);
        }
        else if (nullptr != m_pContainer)
        {
          setDraggingInstance(nullptr);
          m_pContainer->addItem(m_pThis);
        }
        else
        {
          assert(false);
        }
      }
    }
    else if (m_pThis == pObj)
    {
      switch (pEvent->type())
      {
      case QEvent::MouseMove:
        onMouseMove(dynamic_cast<QMouseEvent*>(pEvent));
        break;
      case QEvent::MouseButtonPress:
        onMousePress(dynamic_cast<QMouseEvent*>(pEvent));
        break;
      case QEvent::MouseButtonRelease:
        onMouseRelease(dynamic_cast<QMouseEvent*>(pEvent));
        break;
      default:
        break;
      }
    }

    return false;
  }

private:
  bool m_bMouseDown = false;
  QPoint m_mouseDownPoint;
  T* m_pThis = nullptr;
  static T* m_pDraggingInstance;
  DraggableContainer<T>* m_pContainer = nullptr;
  EventFilter m_eventFilter;
};

#endif // DRAGGABLE_H
