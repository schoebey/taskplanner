#ifndef ODOOREPORT_H
#define ODOOREPORT_H

#include "report.h"

class OdooReport : public Report
{
public:
  OdooReport();

  EReportError create_impl(const Manager& manager) const override;

private:
};

#endif // ODOOREPORT_H
