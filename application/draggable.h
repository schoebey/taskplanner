#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include "flowlayout.h"

#include <QMouseEvent>
#include <QApplication>
#include <QWidget>
#include <QFrame>
#include <QPointer>
#include <QDebug>

#include <vector>
#include <cmath>
#include <cassert>

namespace detail
{
  void sendEnterMoveLeaveEvents(QMouseEvent* pMouseEvent,
                                QPointer<QWidget>& pPreviouslyEntered);
}

//TODO: on drag, set widget to 'transparent for mouse events' and implement normal enter/leave events for widget
//to circumvent increasing processing time with large amounts of draggable containers

enum class EDragMode
{
  eMove,
  eCopy
};

template<typename T>
class DraggableContainer : public QFrame
{
public:
  DraggableContainer(QWidget* pParent)
    : QFrame(pParent)
  {
    setAcceptDrops(true);
  }

  ~DraggableContainer()
  {}

  EDragMode dragMode() const
  {
    return m_dragMode;
  }

  void setDragMode(EDragMode mode)
  {
    m_dragMode = mode;
  }

  bool contains(T* pT) const
  {
    auto it = std::find(m_vpItems.begin(), m_vpItems.end(), pT);
    return it != m_vpItems.end();
  }

  bool addItem(T* pT)
  {
    if (nullptr == pT) { return false; }
    if (addItem_impl(pT))
    {
      pT->setContainer(this);
      if (m_vpItems.end() == std::find(m_vpItems.begin(), m_vpItems.end(), pT))
      {
        m_vpItems.push_back(pT);
      }
      emitItemAdded(pT);
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
      if (m_vpItems.end() == std::find(m_vpItems.begin(), m_vpItems.end(), pT))
      {
        m_vpItems.push_back(pT);
      }
      emitItemInserted(pT, pt);
      return true;
    }
    return false;
  }

  bool removeItem(T* pT)
  {
    if (nullptr == pT) { return false; }
    if (removeItem_impl(pT))
    {
      auto it = std::find(m_vpItems.begin(), m_vpItems.end(), pT);
      if (it != m_vpItems.end())  { m_vpItems.erase(it); }

      emitItemRemoved(pT);
      return true;
    }
    return false;
  }

  bool moveItemFrom(DraggableContainer<T>* pSource, T* pT, QPoint pt)
  {
    if (nullptr == pT || nullptr == pSource) { return false; }

    QPointer<T> pGuard(pT);
    if (moveItemFrom_impl(pSource, pT, pt) && nullptr != pGuard)
    {
      auto it = std::find(pSource->m_vpItems.begin(), pSource->m_vpItems.end(), pT);
      if (it != pSource->m_vpItems.end())  { pSource->m_vpItems.erase(it); }

      pT->setContainer(this);
      if (m_vpItems.end() == std::find(m_vpItems.begin(), m_vpItems.end(), pT))
      {
        m_vpItems.push_back(pGuard);
      }
      emitItemMovedFrom(pT, pSource);
      return true;
    }
    return false;
  }

  bool showPlaceholderAt(const QPoint& pt, const QSize& s)
  {
    if (acceptDrops())
    {
      return showPlaceholderAt_impl(pt, s);
    }

    return false;
  }

  virtual bool showPlaceholderAt_impl(const QPoint&, const QSize&) {}

  virtual void hidePlaceholder() {}

  void enterEvent(QEvent* pEvent) override
  {
    QFrame::enterEvent(pEvent);

    if (acceptDrops())
    {
      for (auto it = m_vpMouseOverContainers.begin();
           it != m_vpMouseOverContainers.end(); ++it)
      {
        if (isAncestorOf(*it))
        {
          m_vpMouseOverContainers.insert(it, this);
          return;
        }
      }

      m_vpMouseOverContainers.push_back(this);
    }
  }

  void leaveEvent(QEvent* pEvent) override
  {
    QFrame::leaveEvent(pEvent);
    auto it = std::find(m_vpMouseOverContainers.begin(),
                        m_vpMouseOverContainers.end(),
                        this);
    if (it != m_vpMouseOverContainers.end())
    {
      m_vpMouseOverContainers.erase(it);
    }
  }

  static DraggableContainer<T>* containerUnderMouse(const QPoint& globalPos)
  {
    // the vector is filled so that parents are located in front of their children,
    // meaning the innermost child is situated at the back of the vector
    auto it = m_vpMouseOverContainers.rbegin();
    while (it != m_vpMouseOverContainers.rend())
    {
      auto pContainer = *it;
      if (pContainer->rect().contains(pContainer->mapFromGlobal(globalPos)))
      {
        return pContainer;
      }
      ++it;
    }

    return nullptr;
  }

  static std::vector<DraggableContainer<T>*> mouseOverContainers()
  {
    return m_vpMouseOverContainers;
  }

  const std::vector<QPointer<T>>& items() const
  {
    return m_vpItems;
  }


private:
  // in order to overcome Qt's MOC limitation with templates, the three
  // potential signals 'added', 'removed', 'moved' are wrapped in (optional)
  // virtual functions that can be implemented if so desired.
  virtual void emitItemAdded(T*) {}
  virtual void emitItemInserted(T*, QPoint) {}
  virtual void emitItemRemoved(T*) {}
  virtual void emitItemMovedFrom(T*, DraggableContainer<T>*) {}

private:
  virtual bool addItem_impl(T* pT) = 0;
  virtual bool removeItem_impl(T* pT) = 0;
  virtual bool insertItem_impl(T* pT, QPoint pt) = 0;
  virtual bool moveItemFrom_impl(DraggableContainer<T>* pSource, T* pT, QPoint pt) = 0;

  static std::vector<DraggableContainer<T>*> m_vpMouseOverContainers;

  std::vector<QPointer<T>> m_vpItems;

  EDragMode m_dragMode = EDragMode::eMove;
};

template<typename T>
class Draggable : public T
{
public:
  template<typename... Args, class C = T, typename std::enable_if<!std::is_copy_constructible<C>::value>::type* = nullptr>
  Draggable(Args... args)
    : T(args...)
  {
    m_fnCallCopyCtor = std::bind([](Args... args, const Draggable<T>*) -> Draggable<T>*
    {
      return new Draggable<T>(args...);
    }, args..., std::placeholders::_1);
  }

  template<typename... Args, class C = T, typename std::enable_if<std::is_copy_constructible<C>::value>::type* = nullptr>
  Draggable(Args... args)
    : T(args...)
  {
    m_fnCallCopyCtor = [](const Draggable<T>* pOther) -> Draggable<T>*
    {
      return new Draggable<T>(*pOther);
    };
  }

  template<class C = T, typename std::enable_if<std::is_copy_constructible<C>::value>::type>
  Draggable(const Draggable<T>& other)
    : T(other),
      m_bMouseDown(other.m_bMouseDown),
      m_mouseDownPoint(other.m_mouseDownPoint),
      m_pContainer(other.m_pContainer)
  {
    m_fnCallCopyCtor = [](const Draggable<T>* pOther) -> Draggable<T>*
    {
      return new Draggable<T>(*pOther);
    };

    T::setMouseTracking(true);
  }

  ~Draggable()
  {}

  EDragMode dragMode() const
  {
    return m_dragMode;
  }

  void setDragMode(EDragMode mode)
  {
    m_dragMode = mode;
  }

  void setMouseDown(bool bMouseDown, QPoint pt = QPoint())
  {
    m_bMouseDown = bMouseDown;
    m_mouseDownPoint = pt;
  }

  void setContainer(DraggableContainer<Draggable>* pContainer)
  {
    m_pContainer = pContainer;
  }

  DraggableContainer<Draggable>* container() const
  {
    return m_pContainer;
  }

  static Draggable<T>* draggingInstance()
  {
    return m_pDraggingInstance;
  }

protected:
  bool mouseDown() const
  {
    return m_bMouseDown;
  }

  QPoint mouseDownPoint() const
  {
    return m_mouseDownPoint;
  }

  static void setDraggingInstance(Draggable<T>* pT)
  {
    m_pDraggingInstance = pT;
  }

  void mouseMoveEvent(QMouseEvent* pMouseEvent) override
  {
    if (mouseDown() && nullptr == draggingInstance())
    {
      QPoint ptDist = pMouseEvent->pos() - mouseDownPoint();
      if (20 < std::sqrt(ptDist.x() * ptDist.x() + ptDist.y() * ptDist.y()))
      {
        // we have to differentiate between two cases:
        // 1. normal drag & drop operation
        // 2. dragging from a 'copy' container, creating a copy of the Draggable
        Draggable<T>* pDraggable = this;

        pDraggable = m_fnCallCopyCtor(this);
        pDraggable->setProperty("reference", QVariant::fromValue(static_cast<void*>(this)));
        setDraggingInstance(pDraggable);

        bool bControlPressed = pMouseEvent->modifiers().testFlag(Qt::ControlModifier);

        EDragMode mode(pDraggable->container()->dragMode());
        if (bControlPressed) {
          mode = EDragMode::eCopy;
        }

        setDragMode(mode);
        T::setVisible(EDragMode::eCopy == dragMode());

        pDraggable->T::setParent(nullptr);
        pDraggable->T::setWindowFlags(Qt::FramelessWindowHint |
                                      Qt::NoDropShadowWindowHint);
        pDraggable->T::setAttribute(Qt::WA_TranslucentBackground, true);
        pDraggable->T::setFocus();
        pDraggable->T::move(pMouseEvent->globalPos() - mouseDownPoint());
        pDraggable->T::show();
        pDraggable->T::raise();
        qApp->installEventFilter(pDraggable);
        pMouseEvent->accept();
      }
    }
  }

  void mousePressEvent(QMouseEvent* pMouseEvent) override
  {
    setMouseDown(true, pMouseEvent->pos());
    T::mousePressEvent(pMouseEvent);
  }

  void mouseReleaseEvent(QMouseEvent* pMouseEvent) override
  {
    setMouseDown(false);
    T::mouseReleaseEvent(pMouseEvent);
  }

  void resizeEvent(QResizeEvent* pEvent) override
  {
    m_mouseDownPoint = T::rect().center();
    T::resizeEvent(pEvent);
  }

  bool eventFilter(QObject* /*pObj*/, QEvent* pEvent) override
  {
    if (this == m_pDraggingInstance)
    {
      if (QEvent::MouseMove == pEvent->type())
      {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
        if (m_lastMousePos != pMouseEvent->globalPos())
        {
          m_lastMousePos = pMouseEvent->globalPos();

          QPoint mappedPt = T::mapFromGlobal(pMouseEvent->globalPos());
          QMouseEvent e(QEvent::Move, mappedPt, pMouseEvent->button(), pMouseEvent->buttons(), pMouseEvent->modifiers());
          T::mouseMoveEvent(&e);

          T::move(T::parentWidget()->mapFromGlobal(pMouseEvent->globalPos() - m_mouseDownPoint));
          if (nullptr != QWidget::mouseGrabber())
          {
            // probably not necessary - mouse events aren't detected
            // by other widgets either way
            QWidget::mouseGrabber()->releaseMouse();
          }
          T::setAttribute(Qt::WA_TransparentForMouseEvents, true);


          for (auto pContainer : DraggableContainer<Draggable>::mouseOverContainers())
          {
            pContainer->hidePlaceholder();
          }


          QPoint pt = pMouseEvent->globalPos();
          DraggableContainer<Draggable>* pContainer = DraggableContainer<Draggable>::containerUnderMouse(pMouseEvent->globalPos());
          if (nullptr != pContainer)
          {
            pt = pContainer->mapFromGlobal(pt);

            pContainer->showPlaceholderAt(pt, T::size());
          }

          detail::sendEnterMoveLeaveEvents(pMouseEvent, m_pPreviouslyEnteredWidget);
        }
      }
      else if (QEvent::MouseButtonRelease == pEvent->type())
      {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
        m_bMouseDown = false;
        DraggableContainer<Draggable>* pContainer = DraggableContainer<Draggable>::containerUnderMouse(pMouseEvent->globalPos());
        QVariant prop = T::property("reference");
        QPointer<Draggable<T>> pOriginal = static_cast<Draggable<T>*>(prop.value<void*>());
        if (nullptr != pContainer)
        {
          QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
          QPoint pt = pMouseEvent->globalPos();
          pt = pContainer->mapFromGlobal(pt);

          // Differentiate between moving a tag or copying it to a new container.
          // Why? copying needs to be undoable by just removing the new tag from the destination container.
          // Moving a tag needs to undo two steps: adding to the new container and removing it from the old one.
          bool bActionSucceeded(false);
          switch (pOriginal->dragMode())
          {
          case EDragMode::eCopy:
            bActionSucceeded = pContainer->insertItemAt(this, pt);
            break;
          case EDragMode::eMove:
            bActionSucceeded = pContainer->moveItemFrom(pOriginal->container(), this, pt);
          default:
            break;
          }

          if (bActionSucceeded)
          {
            setDraggingInstance(nullptr);
          }
          else
          {
            if (nullptr != m_pContainer)
            {
              setDraggingInstance(nullptr);

              if (nullptr != pOriginal )
              {
                pOriginal->setVisible(true);
              }
              T::deleteLater();
            }
          }
        }
        else if (nullptr != m_pContainer)
        {
          setDraggingInstance(nullptr);
          if (nullptr != pOriginal)
          {
            pOriginal->setVisible(true);
          }
          T::deleteLater();
        }
        else
        {
          assert(false);
        }

        for (auto pContainer : DraggableContainer<Draggable>::mouseOverContainers())
        {
          pContainer->hidePlaceholder();
        }

        T::setAttribute(Qt::WA_TransparentForMouseEvents, false);

        qApp->removeEventFilter(this);
      }
    }

    return false;
  }

private:
  bool m_bMouseDown = false;
  QPoint m_mouseDownPoint;
  QPoint m_lastMousePos;
  static Draggable<T>* m_pDraggingInstance;
  DraggableContainer<Draggable<T>>* m_pContainer = nullptr;
  std::function<Draggable<T>*(const Draggable<T>*)> m_fnCallCopyCtor;
  QPointer<QWidget> m_pPreviouslyEnteredWidget = nullptr;
  EDragMode m_dragMode = EDragMode::eMove;
};

#endif // DRAGGABLE_H
