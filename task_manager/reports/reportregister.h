#ifndef REGISTER_REPORT_H
#define REGISTER_REPORT_H

#include "patterns/factory/register.h"
#include "reportinterface.h"

template<typename T> class RegisterReport : public Register<T, IReport, bool>
{
public:
  RegisterReport(const QString& sName)
    : Register<T, IReport, bool>(sName, false)
  {}
};

#endif // REGISTER_REPORT_H
