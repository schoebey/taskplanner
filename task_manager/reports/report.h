#ifndef REPORT_H
#define REPORT_H

#include "libtaskmanager.h"
#include "reportinterface.h"

class LIBTASKMANAGER Report : public IReport
{
public:
  Report();

  EReportError create(const Manager& manager) const override;

protected:
  virtual EReportError create_impl(const Manager& manager) const = 0;
};

#endif // REPORT_H
