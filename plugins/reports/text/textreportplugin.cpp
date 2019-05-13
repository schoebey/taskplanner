#include "textreportplugin.h"

#include <QDebug>

TextReportPlugin::TextReportPlugin()
{

}

void TextReportPlugin::initialize()
{
  qDebug() << "hello from the plugin";
}
