#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QtPlugin>

class IPlugin
{
public:
  virtual ~IPlugin() {}
  virtual void initialize() = 0;

protected:
  IPlugin() {}
};

Q_DECLARE_INTERFACE(IPlugin, "app.taskplanner.plugin")

#endif // PLUGIN_INTERFACE_H
