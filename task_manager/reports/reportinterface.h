#ifndef REPORT_INTERFACE_H
#define REPORT_INTERFACE_H

#include "parametrizable.h"
#include "reportenums.h"

#include <memory>

class Manager;
class IReport : public Parametrizable
{
public:
  virtual ~IReport() {}

  virtual EReportError create(const Manager& manager) const = 0;

protected:
  IReport() {}

};
typedef std::shared_ptr<IReport> tspReport;

#endif // REPORT_INTERFACE_H
