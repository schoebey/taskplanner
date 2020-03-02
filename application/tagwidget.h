#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include <QLabel>
#include <QMouseEvent>
#include <QApplication>

#include <cmath>

template<typename T>
class DraggableContainer
{
public:
  DraggableContainer()
  {}

  ~DraggableContainer()
  {}

  bool addItem(T* pT)
  {
    if (nullptr == pT) { return false; }
    return addItem_impl(pT);
  }

  bool removeItem(T* pT)
  {
    if (nullptr == pT) { return false; }
    return removeItem_impl(pT);
  }

private:
  virtual bool addItem_impl(T* pT) = 0;
  virtual bool removeItem_impl(T* pT) = 0;
};

class A : public DraggableContainer<int>
{
public:
  A() {}
  ~A() {}

  bool addItem_impl(int* pInt) override { return true; }
  bool removeItem_impl(int* pInt) override { return true; }
};

template<typename T>
class Draggable
{
public:
  Draggable(T* pThis)
    : m_pThis(pThis)
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

  void setDraggingInstance(T* pT)
  {
    m_pDraggingInstance = pT;
  }

  T* draggingInstance() const
  {
    return m_pDraggingInstance;
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

        qApp->installEventFilter(m_pThis);
        m_pThis->setParent(m_pThis->window());
        m_pThis->setFocus();
        m_pThis->move(pMouseEvent->globalPos() - mouseDownPoint());
        m_pThis->show();
        m_pThis->raise();
        pMouseEvent->accept();
      }
    }
  }

private:
  bool m_bMouseDown = false;
  QPoint m_mouseDownPoint;
  T* m_pThis = nullptr;
  T* m_pDraggingInstance = nullptr;
  DraggableContainer<T>* m_pContainer = nullptr;
};

class TagWidget : public QLabel, public Draggable<TagWidget>
{
public:
  TagWidget(const QString& sText, QWidget* pParent);
private:
  void paintEvent(QPaintEvent* pEvent) override;
  void mouseMoveEvent(QMouseEvent* pMouseEvent) override;
  void mousePressEvent(QMouseEvent* pMouseEvent) override;
  void mouseReleaseEvent(QMouseEvent* pMouseEvent) override;
};

#endif // TAGWIDGET_H
