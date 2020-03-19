#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include "flowlayout.h"

#include <QMouseEvent>
#include <QApplication>
#include <QWidget>
#include <QFrame>

#include <vector>
#include <cmath>
#include <cassert>

namespace detail
{
  void sendEnterMoveLeaveEvents(QMouseEvent* pMouseEvent,
                                QWidget*& pPreviouslyEntered);
}

//TODO: on drag, set widget to 'transparent for mouse events' and implement normal enter/leave events for widget
//to circumvent increasing processing time with large amounts of draggable containers

enum class EDragMode
{
  eMove,
  eCopy
};

enum class EDropMode
{
  eMoveBackToPreviousContainer,
  eDelete
};

template<typename T>
class DraggableContainer : public QFrame
{
public:
  DraggableContainer(QWidget* pParent)
    : QFrame(pParent)
  {
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

  EDragMode dragMode() const
  {
    return m_dragMode;
  }

  void setDragMode(EDragMode mode)
  {
    m_dragMode = mode;
  }

  EDropMode dropMode() const
  {
    return m_dropMode;
  }

  void setDropMode(EDropMode mode)
  {
    m_dropMode = mode;
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

  virtual bool showPlaceholderAt(const QPoint&, const QSize&) {}

  virtual void hidePlaceholder() {}

  void enterEvent(QEvent* pEvent) override
  {
    QFrame::enterEvent(pEvent);
    m_vpMouseOverContainers.push_back(this);
    setStyleSheet("border: 1px solid red;");
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
      setStyleSheet("");
    }
  }

  static DraggableContainer<T>* containerUnderMouse(const QPoint& globalPos)
  {
    // since the vector is filled from front to back with the newest entered
    // widget at the back, traverse it backwards to get the most restricting
    // container widget first.
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

private:
  virtual bool addItem_impl(T* pT) = 0;
  virtual bool removeItem_impl(T* pT) = 0;
  virtual bool insertItem_impl(T* pT, QPoint pt) = 0;

  static std::vector<DraggableContainer<T>*> m_vpMouseOverContainers;

  EDragMode m_dragMode = EDragMode::eMove;
  EDropMode m_dropMode = EDropMode::eMoveBackToPreviousContainer;
};

template<typename T>
class Draggable : public T
{
public:
  template<typename... Args, class C = T, typename std::enable_if<!std::is_copy_constructible<C>::value>::type* = nullptr>
  Draggable(Args... args)
    : T(args...)
  {
    m_fnCallCopyCtor = std::bind([](Args... args) -> Draggable<T>*
    {
      return new Draggable<T>(args...);
    }, args...);
  }

  template<typename... Args, class C = T, typename std::enable_if<std::is_copy_constructible<C>::value>::type* = nullptr>
  Draggable(Args... args)
    : T(args...)
  {
    m_fnCallCopyCtor = [this]() -> Draggable<T>*
    {
      return new Draggable<T>(*this);
    };
  }

  template<class C = T, typename std::enable_if<std::is_copy_constructible<C>::value>::type>
  Draggable(const Draggable<T>& other)
    : T(other),
      m_bMouseDown(other.m_bMouseDown),
      m_mouseDownPoint(other.m_mouseDownPoint),
      m_pContainer(other.m_pContainer),
      m_fnCallCopyCtor(other.m_fnCallCopyCtor)
  {
    T::setMouseTracking(true);
  }

  ~Draggable()
  {}

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
        if (nullptr != m_pContainer &&
            EDragMode::eCopy == m_pContainer->dragMode())
        {
          pDraggable = m_fnCallCopyCtor();
        }
        setDraggingInstance(pDraggable);

        if (nullptr != m_pContainer)
        {
          m_pContainer->removeItem(pDraggable);
        }

        pDraggable->T::setParent(T::window());
        pDraggable->T::setFocus();
        pDraggable->T::move(T::window()->mapFromGlobal(pMouseEvent->globalPos() - mouseDownPoint()));
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

  bool eventFilter(QObject* /*pObj*/, QEvent* pEvent) override
  {
    if (this == m_pDraggingInstance)
    {
      if (QEvent::MouseMove == pEvent->type())
      {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
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
      else if (QEvent::MouseButtonRelease == pEvent->type())
      {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
        m_bMouseDown = false;
        DraggableContainer<Draggable>* pContainer = DraggableContainer<Draggable>::containerUnderMouse(pMouseEvent->globalPos());
        if (nullptr != pContainer)
        {
          QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
          QPoint pt = pMouseEvent->globalPos();
          pt = pContainer->mapFromGlobal(pt);

          if (pContainer->insertItemAt(this, pt))
          {
            setDraggingInstance(nullptr);
          }
          else
          {
            if (nullptr != m_pContainer)
            {
              setDraggingInstance(nullptr);
              m_pContainer->addItem(this);
            }
          }
        }
        else if (nullptr != m_pContainer)
        {
          setDraggingInstance(nullptr);
          switch (m_pContainer->dropMode())
          {
          case EDropMode::eDelete:
            T::deleteLater();
            break;
          case EDropMode::eMoveBackToPreviousContainer:
          default:
            m_pContainer->addItem(this);
            break;
          }
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
  static Draggable<T>* m_pDraggingInstance;
  DraggableContainer<Draggable<T>>* m_pContainer = nullptr;
  std::function<Draggable<T>*()> m_fnCallCopyCtor;
  QWidget* m_pPreviouslyEnteredWidget = nullptr;
};

#endif // DRAGGABLE_H
