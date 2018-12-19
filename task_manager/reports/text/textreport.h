#ifndef TEXTREPORT_H
#define TEXTREPORT_H

#include "reportinterface.h"
#include "parametrizable.h"

#include <QFile>
#include <QTextStream>

class TextReport : public IReport
{
public:
  TextReport();

private:
  QFile m_file;
  QTextStream m_stream;
};

#endif // TEXTREPORT_H
