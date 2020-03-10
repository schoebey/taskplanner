#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include "flowlayout.h"

#include <QMouseEvent>
#include <QApplication>
#include <QWidget>
#include <QFrame>

#include <set>
#include <cmath>
#include <cassert>

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

  static DraggableContainer<T>* containerUnderMouse(const QPoint& globalPos)
  {
    for (DraggableContainer<T>* pContainer : m_visibleContainers)
    {
      if (pContainer->rect().contains(pContainer->mapFromGlobal(globalPos)))
      {
        return pContainer;
      }
    }

    return nullptr;
  }

  static std::set<DraggableContainer<T>*> visibleContainers()
  {
    return m_visibleContainers;
  }

private:
  virtual bool addItem_impl(T* pT) = 0;
  virtual bool removeItem_impl(T* pT) = 0;
  virtual bool insertItem_impl(T* pT, QPoint pt) = 0;

  void showEvent(QShowEvent* /*pEvent*/) override
  {
    m_visibleContainers.insert(this);
  }

  void hideEvent(QHideEvent* /*pEvent*/) override
  {
    auto it = m_visibleContainers.find(this);
    if (it != m_visibleContainers.end())
    {
      m_visibleContainers.erase(it);
    }
  }

  static std::set<DraggableContainer<T>*> m_visibleContainers;
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
  {}

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

protected:
  bool mouseDown() const
  {
    return m_bMouseDown;
  }

  QPoint mouseDownPoint() const
  {
    return m_mouseDownPoint;
  }

  void setDraggingInstance(Draggable<T>* pT)
  {
    m_pDraggingInstance = pT;
  }

  Draggable<T>* draggingInstance() const
  {
    return m_pDraggingInstance;
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
        bool bCopyMode = true;
        if (bCopyMode)
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
  }

  void mouseReleaseEvent(QMouseEvent* /*pMouseEvent*/) override
  {
    setMouseDown(false);
  }

  bool eventFilter(QObject* /*pObj*/, QEvent* pEvent) override
  {
    if (this == m_pDraggingInstance)
    {
      if (QEvent::MouseMove == pEvent->type())
      {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
        T::move(T::parentWidget()->mapFromGlobal(pMouseEvent->globalPos() - m_mouseDownPoint));

        for (auto pContainer : DraggableContainer<Draggable>::visibleContainers())
        {
          pContainer->hidePlaceholder();
        }

        DraggableContainer<Draggable>* pContainer = DraggableContainer<Draggable>::containerUnderMouse(pMouseEvent->globalPos());
        if (nullptr != pContainer)
        {
          QPoint pt = pMouseEvent->globalPos();
          pt = pContainer->mapFromGlobal(pt);
          pContainer->showPlaceholderAt(pt, T::size());
        }
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
          m_pContainer->addItem(this);
        }
        else
        {
          assert(false);
        }

        for (auto pContainer : DraggableContainer<Draggable>::visibleContainers())
        {
          pContainer->hidePlaceholder();
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
  std::function<Draggable<T>*()> m_fnCallCopyCtor;
};

#endif // DRAGGABLE_H
