#include <gtest/gtest.h>

#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/objectfactory.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <qimessaging/servicedirectory.hpp>


#include "tests/task_proxy.hpp"


class TestTask: public ::testing::Test
{
public:
  TestTask()
  {
    static bool init = false;
    static qi::ObjectPtr tgs;
    if (!init)
    {
      std::vector<std::string> objs = qi::loadObject("taskservice");
      if (objs.size() != 1)
        throw std::runtime_error("Expected one object in taskService");
      tgs = qi::createObject("TaskGeneratorService");
      if (!tgs)
        throw std::runtime_error("No TaskGenerator service found");
      init = true;
    }
    taskGenService = tgs;
  }

protected:
  void SetUp()
  {
    p.server()->registerService("taskGen", taskGenService);
    taskGenClient = p.client()->service("taskGen");
    ASSERT_TRUE(taskGenClient);
    taskGenProxy = new TaskGeneratorProxy(taskGenClient);
  }

  void TearDown()
  {
    taskGenClient.reset();
    delete taskGenProxy;
  }

public:
  TestSessionPair      p;
  qi::ObjectPtr        taskGenService;
  qi::ObjectPtr        taskGenClient;
  TaskGeneratorProxy*  taskGenProxy; // specialized version of taskClient
};

TEST_F(TestTask, Basic)
{
  ASSERT_EQ(0U, taskGenProxy->taskCount());
}

TEST_F(TestTask, Task)
{
  ITaskPtr task = taskGenProxy->newTask("coin");
  ASSERT_TRUE(task);
  std::string n = task->getName();

  ASSERT_EQ(n, "coin");

  unsigned count = taskGenProxy->taskCount();
  ASSERT_EQ(1U, count);

  task->setParam("foo");
  task->step(42);
  std::string lr = task->getLastResult();
  ASSERT_EQ("coin foo 42 1", lr);
  // Release the task, ensure it gets destroyed
  task.reset();
  qi::os::msleep(400);
  count = taskGenProxy->taskCount();
  ASSERT_EQ(0U, count);
}

TEST_F(TestTask, Signals)
{
  ITaskPtr task = taskGenProxy->newTask("coin");
  ASSERT_TRUE(task);
  qi::Promise<std::string> p;
  task->onParamChanged.connect(
    boost::bind(&qi::Promise<std::string>::setValue, p, _1));
  task->setParam("foo");
  ASSERT_EQ("foo", p.future().value());
  std::vector<ITaskPtr> tasks = taskGenProxy->tasks();
  ASSERT_EQ(1U, tasks.size());
  p.reset();
  tasks[0]->setParam("bar");
  ASSERT_EQ("bar", p.future().value());
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  TestMode::forceTestMode(TestMode::Mode_SD);
  TestMode::initTestMode(argc, argv);
  return RUN_ALL_TESTS();
}
