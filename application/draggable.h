#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include <QMouseEvent>
#include <QApplication>
#include <set>

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
  DraggableContainer(QWidget* pThis)
    : m_eventFilter(this),
      m_pThis(pThis)
  {
    qApp->installEventFilter(&m_eventFilter);
  }

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
//    return m_containersUnderMouse.empty() ? nullptr :
//                                            *m_containersUnderMouse.begin();
    return nullptr;
  }

private:
  virtual bool addItem_impl(T* pT) = 0;
  virtual bool removeItem_impl(T* pT) = 0;
  virtual bool insertItem_impl(T* pT, QPoint pt) = 0;

  void onMouseMove(QMouseEvent* pMouseEvent)
  {
//    if (m_pThis->rect().contains(pMouseEvent->pos()))
//    {
//      m_containersUnderMouse.insert(this);
//    }
//    else
//    {
//      auto it = m_containersUnderMouse.find(this);
//      if (it != m_containersUnderMouse.end())
//      {
//        m_containersUnderMouse.erase(it);
//      }
//    }
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

//  static std::set<DraggableContainer<T>*> m_containersUnderMouse;
  EventFilter m_eventFilter;
  QWidget* m_pThis;
};

template<typename T>
class Draggable : public T
{
public:
  template<typename... Args>
  Draggable(Args... args)
    : T(args...)
  {}

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

  void setDraggingInstance(Draggable* pT)
  {
    m_pDraggingInstance = pT;
  }

  Draggable* draggingInstance() const
  {
    return m_pDraggingInstance;
  }

  void setContainer(DraggableContainer<Draggable>* pContainer)
  {
    m_pContainer = pContainer;
  }

  DraggableContainer<Draggable>* container() const
  {
    return m_pContainer;
  }

  void mouseMoveEvent(QMouseEvent* pMouseEvent) override
  {
    if (mouseDown() && nullptr == draggingInstance())
    {
      QPoint ptDist = pMouseEvent->pos() - mouseDownPoint();
      if (20 < std::sqrt(ptDist.x() * ptDist.x() + ptDist.y() * ptDist.y()))
      {
        setDraggingInstance(this);

        if (nullptr != m_pContainer)
        {
          m_pContainer->removeItem(this);
        }

        T::setParent(T::window());
        T::setFocus();
        T::move(pMouseEvent->globalPos() - mouseDownPoint());
        T::show();
        T::raise();
        qApp->installEventFilter(this);
        pMouseEvent->accept();
      }
    }
  }

  void mousePressEvent(QMouseEvent* pMouseEvent) override
  {
    setMouseDown(true, pMouseEvent->pos());
  }

  void mouseReleaseEvent(QMouseEvent* /*pMouseEvent*/) override
  {
    setMouseDown(false);
  }

  bool eventFilter(QObject* pObj, QEvent* pEvent) override
  {
    if (this == m_pDraggingInstance)
    {
      if (QEvent::MouseMove == pEvent->type())
      {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
        T::move(T::parentWidget()->mapFromGlobal(pMouseEvent->globalPos() - m_mouseDownPoint));
      }
      else if (QEvent::MouseButtonRelease == pEvent->type())
      {
        m_bMouseDown = false;
        if (nullptr != DraggableContainer<Draggable>::containerUnderMouse())
        {
          QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
          QWidget* pWidget = dynamic_cast<QWidget*>(this);
          QPoint pt = pMouseEvent->globalPos();
          if (nullptr != pWidget) {
            pt = pWidget->mapFromGlobal(pt);
          }
          DraggableContainer<Draggable>::containerUnderMouse()->insertItemAt(this, pt);
        }
        else if (nullptr != m_pContainer)
        {
          setDraggingInstance(nullptr);
          m_pContainer->addItem(this);
        }
        else
        {
          assert(false);
        }

        qApp->removeEventFilter(this);
      }
    }

    return false;
  }

private:
  bool m_bMouseDown = false;
  QPoint m_mouseDownPoint;
  static Draggable<T>* m_pDraggingInstance;
  DraggableContainer<Draggable<T>>* m_pContainer = nullptr;
};

#endif // DRAGGABLE_H