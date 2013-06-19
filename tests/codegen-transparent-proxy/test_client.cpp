// include generated stuff first to test standalone-ness
#include "task_interface.hpp"

// If I was not linked with service, I would need that
// to make AnyObject -> ITaskPtr conversion available
#include "task_proxy.hpp"

#include "taskservice_proxy.hpp"

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qitype/objectfactory.hpp>
#include <testsession/testsessionpair.hpp>


// client-side task implementation
class CTask: public ITask
{
public:
  CTask() {}
  int addTwo(int i, int j)
  {
    return i + j + value.get();
  }
};

QI_IMPLEMENT_ITask(CTask);

TEST(TransparentProxy, ClientIMpl)
{
  TestSessionPair p;
  p.server()->registerService("builder", qi::createObject("TaskServiceService"));
  qi::AnyObject o = p.client()->service("builder").value()->call<qi::AnyObject>("create");
  EXPECT_TRUE(o);
  TaskServiceProxyPtr pp = qi::GenericValue::from(o).to<TaskServiceProxyPtr>();
  pp->value.set(3);
  EXPECT_TRUE(pp);
  boost::shared_ptr<CTask> task(new CTask());
  pp->accept(task);
  task->value.set(1);
  EXPECT_EQ(9, pp->add(5)); //5 + 3 + 1
  pp->clear();
}

TEST(TransparentProxy, ServerImpl)
{
  TestSessionPair p;
  p.server()->registerService("builder", qi::createObject("TaskServiceService"));
  qi::AnyObject o = p.client()->service("builder").value()->call<qi::AnyObject>("create");
  EXPECT_TRUE(o);
  TaskServiceProxyPtr pp = qi::GenericValue::from(o).to<TaskServiceProxyPtr>();
  pp->value.set(3);
  EXPECT_TRUE(pp);
  ITaskPtr task = pp->make(1);
  // impl add(i, j): i+j + value + ppValue
  // pp->add(i) bounces to taskAdd(i, ppValue())
  // -> i + ppValue()  + ppValue() + value()
  // -> 5 + 3 + 3 + 1
  EXPECT_EQ(12, pp->add(5));
  pp->clear();
}


int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
