#include "libtaskmanager/manager.h"
#include "libtaskmanager/group.h"
#include "libtaskmanager/task.h"

#include "gtest/gtest.h"

namespace {
  class TaskManagerTest : public ::testing::Test {
  protected:
    TaskManagerTest() {
    }

    ~TaskManagerTest() override {
    }

    void SetUp() override {
    }

    void TearDown() override {
    }

    Manager m_manager;
  };
}


TEST_F(TaskManagerTest, AddGroup)
{
  auto pGroup = m_manager.addGroup();
  EXPECT_NE(nullptr, pGroup);
  EXPECT_EQ(pGroup, m_manager.group(pGroup->id()));
}

TEST_F(TaskManagerTest, AddGroupWithId)
{
  auto pGroup = m_manager.addGroup(4711);
  EXPECT_NE(nullptr, pGroup);
  EXPECT_EQ(4711, pGroup->id());
  EXPECT_EQ(pGroup, m_manager.group(pGroup->id()));
}

TEST_F(TaskManagerTest, RemoveGroup)
{
  auto pGroup = m_manager.addGroup();
  ASSERT_NE(nullptr, pGroup);
  EXPECT_TRUE(m_manager.removeGroup(pGroup->id()));
}



TEST_F(TaskManagerTest, AddTask)
{
  auto pTask = m_manager.addTask();
  EXPECT_NE(nullptr, pTask);
  EXPECT_EQ(pTask, m_manager.task(pTask->id()));
}
TEST_F(TaskManagerTest, AddTaskWithId)
{
  auto pTask = m_manager.addTask(4711);
  EXPECT_NE(nullptr, pTask);
  EXPECT_EQ(4711, pTask->id());
  EXPECT_EQ(pTask, m_manager.task(pTask->id()));
}

TEST_F(TaskManagerTest, RemoveTask)
{
  auto pTask = m_manager.addTask();
  ASSERT_NE(nullptr, pTask);
  EXPECT_TRUE(m_manager.removeTask(pTask->id()));
}
