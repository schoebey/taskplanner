#ifndef TEXTREPORTPLUGIN_H
#define TEXTREPORTPLUGIN_H

#include "plugininterface.h"
#include "plugineventbroker.h"

#include <QObject>

class TextReportPlugin : public QObject, public IPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "app.taskplanner.interface")
  Q_INTERFACES(IPlugin)

public:
  TextReportPlugin();

  void initialize(PluginEventBroker* pEventSource) override;
};

#endif // TEXTREPORTPLUGIN_H
