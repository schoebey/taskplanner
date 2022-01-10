#include "odooreport.h"
#include "reportregister.h"
#include "manager.h"
#include "task.h"

#include <QString>
#include <QVariant>
#include <QFile>
#include <QStringList>

namespace
{
  RegisterReport<OdooReport> s("text", "txt");

  static const QString c_sPara_Device = "device";
  static const QString c_sParaStartDate = "startDate";
  static const QString c_sParaEndDate = "endDate";
  static const QString c_sTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";
  static const QString c_sNofDays = "numberOfDays";

  QStringList fullName(const ITask* pTask, const Manager* pManager)
  {
    if (nullptr != pTask)
    {
      auto list = fullName(pManager->task(pTask->parentTask()), pManager);
      list.append(pTask->name());
      return list;
    }

    return QStringList();
  }
}

Q_DECLARE_METATYPE(QIODevice*)

OdooReport::OdooReport()
{
  //registerParameter(c_sPara_FileName, QVariant::String, true);
  registerParameter(c_sPara_Device, QVariant::UserType, true);
  registerParameter(c_sNofDays, QVariant::Int, false);

  registerParameter(...);
}

EReportError OdooReport::create_impl(const Manager& manager) const
{
  depending on option, generate without communication ("dry run")

      this can be used to display in a ui (vector of bookings sorted by date & time)
the ui can optionally display a lineedit for each booking to allow for a comment
      thsi comment can be passed on to the report via option so that the actual report
      can add that comment to the erp system

      the ui should display start and end date and create a live update when params change.
      this has to be done for all reports.


      main application opens an overlay
      main application calls report to write into stream
      displays content of stream,
        * if a task corresponds to a project, the comment field is empty
        * if a task is a *child* of a project, the comment field is filled with its name and/or body text



      create_impl has to have the opportunity to
      * create the report
      * request a dialog where it can insert a custom widget
      * send the report to odoo once the user clicks ok



  // TODO: gather data in necessary format


  // TODO: establish connection to server
  // TODO: get security token
  // TODO: send data


//  QIODevice* pDevice = parameter(c_sPara_Device).value<QIODevice*>();

//  int iNofDays = hasParameter(c_sNofDays) ? parameter(c_sNofDays).value<int>() : 7;

//  if (!pDevice->isOpen() &&
//      !pDevice->open(QIODevice::ReadWrite | QIODevice::Truncate))
//  {
//    return EReportError::eInternalError;
//  }


//  QTextStream s(pDevice);
//  s.setCodec("UTF-8");

//  // generate a report for the past seven days
//  QDateTime startDate(QDate::currentDate().addDays(-iNofDays));
//  QDateTime stopDate(QDate::currentDate().addDays(1));

//  auto numerationFromInt = [](int i) -> QString
//  {
//    QString sNumeration;
//    switch (i % 10)
//    {
//    case 1:  sNumeration = "st";
//      break;
//    case 2:  sNumeration = "nd";
//      break;
//    case 3:  sNumeration = "rd";
//      break;
//    case 0:
//    default: sNumeration = "th";
//      break;
//    }
//    return sNumeration;
//  };

//  // in order to generate the correct header, the stop date has to be altered by -1 day,
//  // else it will point to the next day at 0:00AM
//  QString sFirstLine =
//      QObject::tr("Timesheet for %1 to %2")
//      .arg(startDate.toString(QString("dddd, MMMM dd'%1', yyyy")
//                              .arg(numerationFromInt(startDate.date().day()))))
//      .arg(stopDate.addDays(-1).toString(QString("dddd, MMMM dd'%1', yyyy")
//                                         .arg(numerationFromInt(stopDate.date().day()))));
//  s << sFirstLine << endl;
//  s << QString("=").repeated(sFirstLine.size()) << endl << endl;

//  QDateTime startOfDay = startDate;
//  QDateTime endOfDay = startDate.addDays(1);
//  while (endOfDay <= stopDate)
//  {
//    std::map<QDateTime, std::pair<QDateTime, QString>> timings;

//    for (const auto& taskId : manager.taskIds())
//    {
//      auto pTask = manager.task(taskId);
//      if (nullptr != pTask)
//      {
//        for (const STimeFragment& tf : pTask->timeFragments())
//        {
//          if (tf.stopTime > startOfDay && tf.startTime < endOfDay)
//          {
//            // only count the time of this fragment that was spent within the given interval
//            QDateTime start = std::max<QDateTime>(startOfDay, tf.startTime);
//            QDateTime stop = std::min<QDateTime>(endOfDay, tf.stopTime);

//            QString sName = fullName(pTask, &manager).join(" -> ");

//            timings[start] = std::make_pair(stop, sName);
//          }
//        }
//      }
//    }


//    if (!timings.empty())
//    {
//      s << startOfDay.toString("yyyy-MM-dd") << endl;
//      s << "----------" << endl;

//      for (const auto& el : timings)
//      {
//        s << el.first.toString("hh:mm") << el.second.first.toString(" - hh:mm : ") << el.second.second << endl;
//      }

//      s << endl;
//    }

//    // jump to the next day
//    startOfDay = endOfDay;
//    endOfDay = endOfDay.addDays(1);
//  }

  return EReportError::eOk;
}
