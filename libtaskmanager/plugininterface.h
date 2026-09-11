#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QtPlugin>

class QObject;

class IPlugin
{
public:
  virtual ~IPlugin() {}

  // pEventSource is the application's event broker (a plain QObject so that this
  // libtaskmanager-level interface does not need to depend on any application-layer
  // type); plugins that need type-safe access to its signals can qobject_cast it to
  // the concrete broker type (application/plugineventbroker.h) or connect to it by
  // signal name.
  virtual void initialize(QObject* pEventSource) = 0;

protected:
  IPlugin() {}
};

Q_DECLARE_INTERFACE(IPlugin, "app.taskplanner.plugin")

#endif // PLUGIN_INTERFACE_H
