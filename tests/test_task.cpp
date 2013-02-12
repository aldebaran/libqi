#include <boost/lexical_cast.hpp>

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
      std::vector<std::string> objs = qi::loadObject("task");
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
  tasks.clear();
  task.reset();
  qi::os::msleep(400);
  unsigned count = taskGenProxy->taskCount();
  ASSERT_EQ(0U, count);
}

struct Context
{
  qi::Session*          session;
  qi::ObjectPtr         taskGenClient;
  TaskGeneratorProxy*   taskGenProxy;
  std::vector<ITaskPtr> tasks;
};

void on_step(const std::string& s, unsigned& i)
{
  //std::cerr << "##STEP " << s << std::endl;
  ++i;
}

TEST_F(TestTask, ManyManyTasks)
{
  long long time = qi::os::ustime();
  unsigned NCLIENTS = 10;
  unsigned NTASKS = 10;
  if (getenv("NCLIENTS"))
    NCLIENTS = strtol(getenv("NCLIENTS"), 0, 0);
  if (getenv("NTASKS"))
    NTASKS = strtol(getenv("NTASKS"), 0, 0);
  unsigned TOTAL_CLIENTS = NTASKS * NCLIENTS;
  std::vector<Context> clients;
  for (unsigned i=0; i<NCLIENTS; ++i)
  {
    Context c;
    c.session = new qi::Session();
    c.session->connect(p.serviceDirectoryEndpoints()[0]);
    c.taskGenClient = c.session->service("taskGen");
    c.taskGenProxy = new TaskGeneratorProxy(taskGenClient);
    clients.push_back(c);
  }
  qiLogInfo() << "Setup clients: " << (qi::os::ustime() - time) << std::endl;
  time = qi::os::ustime();
  unsigned counter = 0;
  for (unsigned i=0; i<NCLIENTS; ++i)
  {
    for (unsigned j=0; j<NTASKS; ++j)
    {
      ITaskPtr task = clients[i].taskGenProxy->newTask(
        boost::lexical_cast<std::string>(i*NTASKS + j));
      clients[i].tasks.push_back(task);
      task->onStep.connect(
        boost::bind(on_step, _1, boost::ref(counter)));
    }
  }
  ASSERT_EQ(NCLIENTS*NTASKS, taskGenProxy->taskCount());
  qiLogInfo() << "Setup tasks: " << (qi::os::ustime() - time) << std::endl;
  time = qi::os::ustime();
  taskGenProxy->step(1);
  for (unsigned step=0; step<1000 && counter < TOTAL_CLIENTS; ++step)
    qi::os::msleep(5);
  ASSERT_EQ(counter, TOTAL_CLIENTS);
  std::cerr << "Run: " << (qi::os::ustime() - time) << std::endl;
  // Teardown
  for (unsigned i=0; i<NCLIENTS; ++i)
  {
    delete clients[i].session;
  }
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  TestMode::forceTestMode(TestMode::Mode_SD);
  TestMode::initTestMode(argc, argv);
  return RUN_ALL_TESTS();
}
