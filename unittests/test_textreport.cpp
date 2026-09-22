#include "plugins/reports/text/textreport.h"

#include "libtaskmanager/manager.h"
#include "libtaskmanager/task.h"

#include "gtest/gtest.h"

#include <QBuffer>
#include <QDateTime>
#include <QString>
#include <QVariant>

Q_DECLARE_METATYPE(QIODevice*)

namespace {

  // creates the report and returns its full textual output
  QString runReport(TextReport& report, const Manager& manager, QIODevice& device)
  {
    bool bParamSet = report.setParameter("device", QVariant::fromValue<QIODevice*>(&device));
    EXPECT_TRUE(bParamSet);

    EReportError err = report.create(manager);
    EXPECT_EQ(EReportError::eOk, err);

    device.seek(0);
    return QString::fromUtf8(device.readAll());
  }

}

// reproduces the bug where two tasks' time fragments that clip to the same
// start timestamp caused one of them to silently disappear from the report
// because the report grouped fragments in a std::map keyed by start time.
TEST(TextReportTest, CoincidentStartTimesAreBothReported)
{
  Manager manager;

  ITask* pTask1 = manager.addTask();
  ASSERT_NE(nullptr, pTask1);
  pTask1->setName("Alpha");

  ITask* pTask2 = manager.addTask();
  ASSERT_NE(nullptr, pTask2);
  pTask2->setName("Beta");

  QDateTime start = QDateTime::currentDateTime().addSecs(-3600);

  // both tasks have a fragment that starts at the exact same timestamp
  pTask1->startWork(start);
  pTask1->stopWork(start.addSecs(600));

  pTask2->startWork(start);
  pTask2->stopWork(start.addSecs(1200));

  TextReport report;
  QBuffer buffer;

  QString sOutput = runReport(report, manager, buffer);

  EXPECT_EQ(1, sOutput.count("Alpha"));
  EXPECT_EQ(1, sOutput.count("Beta"));
}

// reproduces the bug where an actively-tracked (still open) time fragment
// was unconditionally excluded from the report because the comparison
// against the invalid/null stopTime always failed.
TEST(TextReportTest, ActivelyTrackedFragmentIsReported)
{
  Manager manager;

  ITask* pTask = manager.addTask();
  ASSERT_NE(nullptr, pTask);
  pTask->setName("Gamma");

  QDateTime start = QDateTime::currentDateTime().addSecs(-120);

  // start work but never stop it -> open/actively-tracked fragment
  pTask->startWork(start);
  ASSERT_TRUE(pTask->isTrackingTime());

  TextReport report;
  QBuffer buffer;

  QString sOutput = runReport(report, manager, buffer);

  EXPECT_EQ(1, sOutput.count("Gamma"));
}
