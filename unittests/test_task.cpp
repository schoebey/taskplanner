#include "libtaskmanager/manager.h"
#include "libtaskmanager/group.h"
#include "libtaskmanager/task.h"

#include "gtest/gtest.h"

namespace {
class TaskTest : public ::testing::Test {
protected:
  TaskTest() {
  }

  ~TaskTest() override {
  }

  void SetUp() override {
    m_pTask0 = m_manager.addTask(0); m_pTask0->setName("a");
    m_pTask1 = m_manager.addTask(1); m_pTask1->setName("b");
    m_pTask2 = m_manager.addTask(2); m_pTask2->setName("c");

    m_pGroup0 = m_manager.addGroup(0); m_pGroup0->setName("group a");
    m_pGroup1 = m_manager.addGroup(1); m_pGroup1->setName("group b");
    m_pGroup2 = m_manager.addGroup(2); m_pGroup2->setName("group c");
  }

  void TearDown() override {
  }

  Manager m_manager;
  ITask* m_pTask0 = nullptr;
  ITask* m_pTask1 = nullptr;
  ITask* m_pTask2 = nullptr;
  IGroup* m_pGroup0 = nullptr;
  IGroup* m_pGroup1 = nullptr;
  IGroup* m_pGroup2 = nullptr;
};
class TimedTaskTest : public TaskTest {
protected:
  TimedTaskTest() {
  }

  ~TimedTaskTest() override {
  }

  void SetUp() override {
    TaskTest::SetUp();

    m_start = QDateTime::fromString("2000-05-05T10:00:00", Qt::ISODate);
    m_start2 = QDateTime::fromString("2000-05-05T12:00:00", Qt::ISODate);
    m_start3 = QDateTime::fromString("2000-05-05T14:00:00", Qt::ISODate);
    m_end = m_start.addSecs(1800);
    m_end2 = m_start2.addSecs(1800);
    m_end3 = m_start3.addSecs(1800);

    // start the first fragment
    m_pTask0->startWork(m_start);
    // and stop it 30mins later
    m_pTask0->stopWork(m_end);

    // start the second fragment
    m_pTask0->startWork(m_start2);
    // and stop it 30mins later
    m_pTask0->stopWork(m_end2);

    // start the second fragment
    m_pTask0->startWork(m_start3);
    // and stop it 30mins later
    m_pTask0->stopWork(m_end3);
  }

  void TearDown() override {
  }


  QDateTime m_start;
  QDateTime m_start2;
  QDateTime m_start3;
  QDateTime m_end;
  QDateTime m_end2;
  QDateTime m_end3;
};
}


TEST_F(TaskTest, SetParentTask)
{
  ASSERT_NE(nullptr, m_pTask0);
  ASSERT_NE(nullptr, m_pTask1);
  ASSERT_NE(nullptr, m_pTask2);

  m_pTask0->setParentTask(m_pTask1->id());
  EXPECT_EQ(m_pTask1->id(), m_pTask0->parentTask());

  m_pTask1->setParentTask(m_pTask2->id());
  EXPECT_EQ(m_pTask2->id(), m_pTask1->parentTask());

  ASSERT_EQ(1u, m_pTask1->taskIds().size());
  EXPECT_EQ(m_pTask0->id(), *m_pTask1->taskIds().begin());

  ASSERT_EQ(1u, m_pTask2->taskIds().size());
  EXPECT_EQ(m_pTask1->id(), *m_pTask2->taskIds().begin());
}

TEST_F(TaskTest, AddTask)
{
  EXPECT_TRUE(m_pTask1->addTask(m_pTask0->id()));
  EXPECT_EQ(m_pTask0->parentTask(), m_pTask1->id());
  ASSERT_NE(0u, m_pTask1->taskIds().size());
  EXPECT_EQ(m_pTask0->id(), *m_pTask1->taskIds().begin());
}

TEST_F(TaskTest, AddTask_AutoRemoveIfChildOfOther)
{
  m_pTask2->addTask(m_pTask0->id());
  EXPECT_EQ(1u, m_pTask2->taskIds().size());

  EXPECT_TRUE(m_pTask1->addTask(m_pTask0->id()));
  EXPECT_EQ(m_pTask0->parentTask(), m_pTask1->id());

  // if adding the task to m_pTask1 was successful,
  // it should have been removed automatically from m_pTask2
  EXPECT_EQ(0u, m_pTask2->taskIds().size());
}

TEST_F(TaskTest, RemoveTask)
{
  m_pTask1->addTask(m_pTask0->id());
  EXPECT_TRUE(m_pTask1->removeTask(m_pTask0->id()));
  EXPECT_EQ(0u, m_pTask1->taskIds().size());
}

TEST_F(TaskTest, RemoveTask_FailIfNoSubTask)
{
  // trying to remove a task that is no subtask should fail
  EXPECT_FALSE(m_pTask0->removeTask(m_pTask1->id()));

  m_pTask1->addTask(m_pTask0->id());
  EXPECT_TRUE(m_pTask1->removeTask(m_pTask0->id()));
  EXPECT_EQ(0u, m_pTask1->taskIds().size());
}

TEST_F(TaskTest, RemoveTask_FailIfNoDirectDescendant)
{
  // trying to remove a task that is no subtask should fail
  EXPECT_FALSE(m_pTask0->removeTask(m_pTask1->id()));

  m_pTask1->addTask(m_pTask0->id());
  m_pTask2->addTask(m_pTask1->id());

  // trying to remove a task that is a non-direct descendant
  // should fail.
  EXPECT_FALSE(m_pTask2->removeTask(m_pTask0->id()));
}

TEST_F(TaskTest, setGroup)
{
  m_pTask0->setGroup(m_pGroup0->id());

  ASSERT_NE(0u, m_pGroup0->taskIds().size());
  EXPECT_EQ(m_pTask0->id(), *m_pGroup0->taskIds().begin());
  EXPECT_EQ(m_pGroup0->id(), m_pTask0->group());
}

TEST_F(TaskTest, setGroup_AutoRemoveIfInDifferentGroup)
{
  m_pTask0->setGroup(m_pGroup1->id());
  EXPECT_EQ(1u, m_pGroup1->taskIds().size());

  m_pTask0->setGroup(m_pGroup0->id());
  EXPECT_EQ(0u, m_pGroup1->taskIds().size());

  ASSERT_EQ(1u, m_pGroup0->taskIds().size());
  EXPECT_EQ(m_pTask0->id(), *m_pGroup0->taskIds().begin());
  EXPECT_EQ(m_pGroup0->id(), m_pTask0->group());
}

TEST_F(TaskTest, setPriority)
{
  SPriority pri;
  pri.setPriority(0, 10);
  m_pTask0->setPriority(pri);

  SPriority pri2;
  pri2.setPriority(0,5);
  m_pTask1->setPriority(pri2);

  EXPECT_EQ(10, m_pTask0->priority().priority(0));
  EXPECT_EQ(5, m_pTask1->priority().priority(0));

  // adding tasks to groups should change their priority
  m_pGroup0->addTask(m_pTask0->id());
  m_pGroup0->addTask(m_pTask1->id());

  EXPECT_EQ(0, m_pTask0->priority().priority(0));
  EXPECT_EQ(1, m_pTask1->priority().priority(0));
}

TEST_F(TaskTest, startWork)
{
  m_pTask0->startWork();

  EXPECT_TRUE(m_pTask0->isTrackingTime());

  m_pTask1->startWork();

  // starting time tracking in one task should not automatically
  // stop time tracking in another - that's up to the application
  // to decide.
  EXPECT_TRUE(m_pTask0->isTrackingTime());
  EXPECT_TRUE(m_pTask1->isTrackingTime());
}

TEST_F(TaskTest, stopWork)
{
  m_pTask0->startWork();

  EXPECT_TRUE(m_pTask0->isTrackingTime());

  m_pTask0->stopWork();

  EXPECT_FALSE(m_pTask0->isTrackingTime());
}

TEST_F(TaskTest, startStopWork_withSpecificTimepoints)
{
  const QDateTime startDateTime = QDateTime::fromString("2000-01-01T12:00:00", Qt::ISODate);
  const QDateTime stopDateTime = QDateTime::fromString("2000-01-01T13:00:00", Qt::ISODate);
  m_pTask0->startWork(startDateTime);
  m_pTask0->stopWork(stopDateTime);

  auto timeFragments = m_pTask0->timeFragments();
  ASSERT_EQ(1u, timeFragments.size());

  EXPECT_EQ(startDateTime, timeFragments[0].startTime);
  EXPECT_EQ(stopDateTime, timeFragments[0].stopTime);
}

TEST_F(TimedTaskTest, insertTimeFragment_isolated)
{
  EXPECT_EQ(3u, m_pTask0->timeFragments().size());

  // insert a time fragment that stands on its own
  m_pTask0->insertTimeFragment(m_end.addSecs(300), m_end.addSecs(600));

  EXPECT_EQ(4u, m_pTask0->timeFragments().size());
}

TEST_F(TimedTaskTest, insertTimeFragment_disappearIfFullyContainedWithinPreExisting)
{
  auto vFragments = m_pTask0->timeFragments();

  // insert a time fragment that lies within a preexisting fragment
  m_pTask0->insertTimeFragment(m_start.addSecs(60), m_end.addSecs(-60));

  // the time fragments of the task should not have changed,
  // since the freshly inserted one lies within a preexisting one
  EXPECT_EQ(3u, m_pTask0->timeFragments().size());
  EXPECT_EQ(vFragments, m_pTask0->timeFragments());
}

TEST_F(TimedTaskTest, insertTimeFragment_extendPreExistingIfOverlapOnLeft)
{
  auto vFragments = m_pTask0->timeFragments();

  // insert a time fragment that starts before the first preexisting fragment
  // and extends into it
  m_pTask0->insertTimeFragment(m_start.addSecs(-300), m_start.addSecs(60));

  // the amount of time fragments of the task should not have changed
  auto vNewFragments = m_pTask0->timeFragments();
  EXPECT_EQ(3u, vNewFragments.size());

  // the first fragment should now be larger
  auto oldFragment = vFragments[0];
  auto newFragment = vNewFragments[0];
  EXPECT_EQ(300, newFragment.startTime.secsTo(oldFragment.startTime));
  EXPECT_EQ(oldFragment.stopTime, newFragment.stopTime);
}

TEST_F(TimedTaskTest, insertTimeFragment_extendPreExistingIfOverlapOnRight)
{
  auto vFragments = m_pTask0->timeFragments();

  // insert a time fragment that starts inside the first preexisting fragment
  // and stops after it
  m_pTask0->insertTimeFragment(m_start.addSecs(30), m_end.addSecs(300));

  // the amount of time fragments of the task should not have changed
  auto vNewFragments = m_pTask0->timeFragments();
  EXPECT_EQ(3u, vNewFragments.size());

  // the first fragment should now be larger
  auto oldFragment = vFragments[0];
  auto newFragment = vNewFragments[0];
  EXPECT_EQ(300, oldFragment.stopTime.secsTo(newFragment.stopTime));
  EXPECT_EQ(oldFragment.startTime, newFragment.startTime);
}

TEST_F(TimedTaskTest, insertTimeFragment_extendPreExistingIfOverlapOnBothSides)
{
  auto vFragments = m_pTask0->timeFragments();

  // insert a time fragment that starts before the first preexisting fragment
  // and stops after it
  m_pTask0->insertTimeFragment(m_start.addSecs(-300), m_end.addSecs(300));

  // the amount of time fragments of the task should not have changed
  auto vNewFragments = m_pTask0->timeFragments();
  EXPECT_EQ(3u, vNewFragments.size());

  // the first fragment should now be larger
  auto oldFragment = vFragments[0];
  auto newFragment = vNewFragments[0];
  EXPECT_EQ(300, oldFragment.stopTime.secsTo(newFragment.stopTime));
  EXPECT_EQ(300, newFragment.startTime.secsTo(oldFragment.startTime));
}

TEST_F(TimedTaskTest, insertTimeFragment_mergeIfOverlapCoversGap)
{
  auto vFragments = m_pTask0->timeFragments();

  // insert a time fragment that starts inside the first preexisting fragment
  // and extends into the second preexisting fragment
  m_pTask0->insertTimeFragment(m_start.addSecs(50), m_end.addSecs(5500));

  // the amount of time fragments of the task should have changed
  auto vNewFragments = m_pTask0->timeFragments();
  EXPECT_EQ(2u, vNewFragments.size());

  // the first fragment should now be larger
  auto oldFragment = vFragments[0];
  auto oldFragment1 = vFragments[1];
  auto newFragment = vNewFragments[0];
  EXPECT_EQ(newFragment.startTime, oldFragment.startTime);
  EXPECT_EQ(newFragment.stopTime, oldFragment1.stopTime);
}

TEST_F(TimedTaskTest, removeimeFragment_isolated)
{
  auto vFragments = m_pTask0->timeFragments();
  EXPECT_EQ(3u, vFragments.size());

  // remove a possibel time fragment that lies between two existing fragments
  m_pTask0->removeTimeFragment(m_end.addSecs(300), m_end.addSecs(120));

  // since the time to be removed does not touch any existing fragments,
  // nothing should have been altered
  EXPECT_EQ(vFragments, m_pTask0->timeFragments());
}

TEST_F(TimedTaskTest, removeimeFragment_shrinkIfOverlapOnLeft)
{
  auto vFragments = m_pTask0->timeFragments();
  EXPECT_EQ(3u, vFragments.size());

  // remove a possible time fragment that partially overlaps the left part of an existing one
  m_pTask0->removeTimeFragment(m_start.addSecs(-300), m_start.addSecs(300));

  EXPECT_NE(vFragments, m_pTask0->timeFragments());

  vFragments = m_pTask0->timeFragments();
  EXPECT_EQ(3u, vFragments.size());

  EXPECT_EQ(m_start.addSecs(300), vFragments[0].startTime);
  EXPECT_EQ(m_end, vFragments[0].stopTime);
}

TEST_F(TimedTaskTest, removeimeFragment_shrinkIfOverlapOnRight)
{
  auto vFragments = m_pTask0->timeFragments();
  EXPECT_EQ(3u, vFragments.size());

  // remove a possible time fragment that partially overlaps the right part of an existing one
  m_pTask0->removeTimeFragment(m_end.addSecs(-300), m_end.addSecs(300));

  EXPECT_NE(vFragments, m_pTask0->timeFragments());

  vFragments = m_pTask0->timeFragments();
  EXPECT_EQ(3u, vFragments.size());

  EXPECT_EQ(m_end.addSecs(-300), vFragments[0].stopTime);
  EXPECT_EQ(m_start, vFragments[0].startTime);
}

TEST_F(TimedTaskTest, removeimeFragment_splitIfFullyContained)
{
  auto vFragments = m_pTask0->timeFragments();
  EXPECT_EQ(3u, vFragments.size());

  // remove a possible time fragment that lies in the middle of an existing one
  m_pTask0->removeTimeFragment(m_start.addSecs(300), m_end.addSecs(-300));

  EXPECT_NE(vFragments, m_pTask0->timeFragments());

  vFragments = m_pTask0->timeFragments();
  EXPECT_EQ(4u, vFragments.size());

  // test the left part of the overlap
  EXPECT_EQ(m_start, vFragments[0].startTime);
  EXPECT_EQ(m_start.addSecs(300), vFragments[0].stopTime);

  // test the right part of the overlap
  EXPECT_EQ(m_end.addSecs(-300), vFragments[1].startTime);
  EXPECT_EQ(m_end, vFragments[1].stopTime);
}







