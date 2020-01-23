#ifndef REGISTER_REPORT_H
#define REGISTER_REPORT_H

#include "patterns/factory/register.h"
#include "reportinterface.h"

template<typename T> class RegisterReport : public Register<T, IReport, QString>
{
public:
  RegisterReport(const QString& sName, const QString& sExtension)
    : Register<T, IReport, QString>(sName, sExtension)
  {}
};

#endif // REGISTER_REPORT_H
