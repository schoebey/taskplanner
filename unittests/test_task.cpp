#include "task_manager/manager.h"
#include "task_manager/group.h"
#include "task_manager/task.h"

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
    ITask* m_pTask0;
    ITask* m_pTask1;
    ITask* m_pTask2;
    IGroup* m_pGroup0;
    IGroup* m_pGroup1;
    IGroup* m_pGroup2;
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
  ASSERT_NE(0, m_pTask1->taskIds().size());
  EXPECT_EQ(m_pTask0->id(), *m_pTask1->taskIds().begin());
}

TEST_F(TaskTest, AddTask_AutoRemoveIfChildOfOther)
{
  m_pTask2->addTask(m_pTask0->id());
  EXPECT_EQ(1, m_pTask2->taskIds().size());

  EXPECT_TRUE(m_pTask1->addTask(m_pTask0->id()));
  EXPECT_EQ(m_pTask0->parentTask(), m_pTask1->id());

  // if adding the task to m_pTask1 was successful,
  // it should have been removed automatically from m_pTask2
  EXPECT_EQ(0, m_pTask2->taskIds().size());
}

TEST_F(TaskTest, RemoveTask)
{
  m_pTask1->addTask(m_pTask0->id());
  EXPECT_TRUE(m_pTask1->removeTask(m_pTask0->id()));
  EXPECT_EQ(0, m_pTask1->taskIds().size());
}

TEST_F(TaskTest, RemoveTask_FailIfNoSubTask)
{
  // trying to remove a task that is no subtask should fail
  EXPECT_FALSE(m_pTask0->removeTask(m_pTask1->id()));

  m_pTask1->addTask(m_pTask0->id());
  EXPECT_TRUE(m_pTask1->removeTask(m_pTask0->id()));
  EXPECT_EQ(0, m_pTask1->taskIds().size());
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

  ASSERT_NE(0, m_pGroup0->taskIds().size());
  EXPECT_EQ(m_pTask0->id(), *m_pGroup0->taskIds().begin());
  EXPECT_EQ(m_pGroup0->id(), m_pTask0->group());
}

TEST_F(TaskTest, setGroup_AutoRemoveIfInDifferentGroup)
{
  m_pTask0->setGroup(m_pGroup1->id());
  EXPECT_EQ(1u, m_pGroup1->taskIds().size());

  m_pTask0->setGroup(m_pGroup0->id());
  EXPECT_EQ(0, m_pGroup1->taskIds().size());

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

