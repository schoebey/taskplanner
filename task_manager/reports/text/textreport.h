#ifndef TEXTREPORT_H
#define TEXTREPORT_H

#include "report.h"

class TextReport : public Report
{
public:
  TextReport();

  EReportError create_impl(const Manager& manager) const override;

private:
};

#endif // TEXTREPORT_H
