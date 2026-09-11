#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QtPlugin>

class PluginEventBroker;

class IPlugin
{
public:
  virtual ~IPlugin() {}

  // pEventSource is the application's event broker. Its concrete type is used (rather
  // than a plain QObject) so that plugins can use type-safe pointer-to-member connects,
  // e.g. connect(pEventSource, &PluginEventBroker::alert, ...), instead of the old-style
  // string-based SIGNAL()/SLOT() macro connects.
  virtual void initialize(PluginEventBroker* pEventSource) = 0;

protected:
  IPlugin() {}
};

Q_DECLARE_INTERFACE(IPlugin, "app.taskplanner.plugin")

#endif // PLUGIN_INTERFACE_H
