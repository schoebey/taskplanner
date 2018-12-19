#ifndef REPORT_INTERFACE_H
#define REPORT_INTERFACE_H

#include "parametrizable.h"

#include <memory>

class IReport : public Parametrizable
{
public:
  virtual ~IReport() {}

protected:
  IReport() {}

};
typedef std::shared_ptr<IReport> tspReport;

#endif // REPORT_INTERFACE_H
