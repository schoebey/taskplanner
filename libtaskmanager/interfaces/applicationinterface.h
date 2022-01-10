#ifndef APPLICATIONINTERFACE_H
#define APPLICATIONINTERFACE_H

class QWidget;
class IOverlay;
class IApplication
{
public:
  /*!
   * \brief requestOverlay requests the display of an overlay that shows \a pEmbeddedWidget
   * \param pEmbeddedWidget widget that should be embedded in the overlay.
   * The overlay will take ownership of the widget. Upon closing of the overlay, the embedded
   * widget will be destroyed.
   */
  IOverlay* requestOverlay(QWidget* pEmbeddedWidget) const;

protected:
  IApplication() = default;
  ~IApplication() = default;
};

#endif // APPLICATIONINTERFACE_H
