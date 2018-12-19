#include "textreport.h"
#include "patterns/factory/register.h"
#include "patterns/factory/registrar.h"
#include "group.h"
#include "task.h"

#include <QString>
#include <QVariant>
#include <QFile>

namespace
{
//  Register<TextReport, IReport> s("text");

  static const QString c_sPara_FileName = "fileName";
  static const QString c_sManagerHeader = "== manager ==";
  static const QString c_sTaskHeader = "== task ==";
  static const QString c_sGroupHeader = "== group ==";
  static const QString c_sTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";
}

TextReport::TextReport()
{
  registerParameter(c_sPara_FileName, QVariant::String, true);
}


