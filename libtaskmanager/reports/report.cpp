#include "report.h"

Report::Report()
{
}

EReportError Report::create(const Manager& manager) const
{
  if (!checkRequiredParameters())
  {
    return EReportError::eParameterMissing;
  }

  return create_impl(manager);
}
