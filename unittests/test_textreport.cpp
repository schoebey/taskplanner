#include "plugins/reports/text/textreport.h"

#include "libtaskmanager/manager.h"
#include "libtaskmanager/task.h"

#include "gtest/gtest.h"

#include <QBuffer>
#include <QDateTime>
#include <QRegularExpression>
#include <QString>
#include <QTime>
#include <QVariant>

#include <cstdlib>

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

  // the emitted range should start at "start" and stop close to "now",
  // clipped correctly instead of being empty/inverted
  QRegularExpression re("(\\d{2}:\\d{2}) - (\\d{2}:\\d{2}) : Gamma");
  QRegularExpressionMatch match = re.match(sOutput);
  ASSERT_TRUE(match.hasMatch());

  EXPECT_EQ(start.toString("hh:mm"), match.captured(1));

  // allow for wall-clock drift between test setup and report generation
  QTime stopTime = QTime::fromString(match.captured(2), "hh:mm");
  QTime nowTime = QDateTime::currentDateTime().time();
  EXPECT_LE(std::abs(stopTime.secsTo(nowTime)), 120);
}

// reproduces the fix for the case where an open (actively-tracked) fragment's
// start time got snapped into the future - e.g. by Task::removeTimeFragment()
// reacting to another task's fragment being inserted with an unvalidated
// future end time (see MainWindow::onAddTimeToTaskRequested). The report must
// not emit an inverted/garbage "start - stop" line for such a fragment.
TEST(TextReportTest, FutureStartOpenFragmentIsNotReported)
{
  Manager manager;

  ITask* pTask = manager.addTask();
  ASSERT_NE(nullptr, pTask);
  pTask->setName("Delta");

  QDateTime start = QDateTime::currentDateTime().addSecs(-120);
  QDateTime future = QDateTime::currentDateTime().addSecs(120);

  // start work but never stop it -> open/actively-tracked fragment
  pTask->startWork(start);
  ASSERT_TRUE(pTask->isTrackingTime());

  // simulate another task's fragment being inserted with an unvalidated
  // future end time, which snaps this open fragment's start time to it
  pTask->removeTimeFragment(start, future);

  auto vFragments = pTask->timeFragments();
  ASSERT_EQ(1u, vFragments.size());
  EXPECT_EQ(future, vFragments[0].startTime);
  EXPECT_FALSE(vFragments[0].stopTime.isValid());

  TextReport report;
  QBuffer buffer;

  QString sOutput = runReport(report, manager, buffer);

  // the fragment's effective stop ("now") is not after its (future) start,
  // so no line must be emitted for it
  EXPECT_EQ(0, sOutput.count("Delta"));
}
