#include "textserializerplugin.h"

#include <QDebug>

TextSerializerPlugin::TextSerializerPlugin()
{

}

void TextSerializerPlugin::initialize()
{
  qDebug() << "hello from the plugin";
}
