#ifndef MARKDOWNSERIALIZERPLUGIN_H
#define MARKDOWNSERIALIZERPLUGIN_H

#include "plugininterface.h"
#include "plugineventbroker.h"

#include <QObject>

class MarkdownSerializerPlugin : public QObject, public IPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "app.taskplanner.interface")
  Q_INTERFACES(IPlugin)

public:
  MarkdownSerializerPlugin();

  void initialize(PluginEventBroker* pEventSource) override;
};

#endif // MARKDOWNSERIALIZERPLUGIN_H
