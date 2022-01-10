#ifndef PLUGIN_H
#define PLUGIN_H

#include "plugininterface.h"

#include <QObject>

class Plugin : public QObject, public IPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "app.taskplanner.interface")
  Q_INTERFACES(IPlugin)

public:
  Plugin();

  void initialize() override;
};

#endif // PLUGIN_H
