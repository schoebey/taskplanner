#ifndef REPORTFACTORY_H
#define REPORTFACTORY_H

#include "reportinterface.h"
#include "patterns/factory/factory.h"

#include <QString>

class ReportFactory : public Factory<IReport, QString>
{};

#endif // REPORTFACTORY_H
