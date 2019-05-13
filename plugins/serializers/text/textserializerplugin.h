#ifndef TEXTSERIALIZERPLUGIN_H
#define TEXTSERIALIZERPLUGIN_H

#include "plugininterface.h"

#include <QObject>

class TextSerializerPlugin : public QObject, public IPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "app.taskplanner.interface")
  Q_INTERFACES(IPlugin)

public:
  TextSerializerPlugin();

  void initialize() override;
};

#endif // TEXTSERIALIZERPLUGIN_H
