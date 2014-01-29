#include <cat.hpp>
#include <qitype/objectfactory.hpp>
#include <qitype/dynamicobject.hpp>
#include <qi/application.hpp>
#include <gtest/gtest.h>

using qi::AnyObject;
using qi::SignalLink;
using qi::GenericFunctionParameters;
using qi::SignalSubscriber;

using animals::CatAction;

// Make a dummy bouncer to test proxy mode
class DummyBouncer: public qi::DynamicObject
{
public:

  DummyBouncer(qi::AnyObject tgt = qi::AnyObject())
  : _target(tgt) {
    if (tgt)
      setMetaObject(tgt.metaObject());
  }
  virtual qi::Future<qi::AnyReference> metaCall(qi::AnyObject context, unsigned int method, const qi::GenericFunctionParameters& params, qi::MetaCallType callType = qi::MetaCallType_Auto)
  {
    return _target.metaCall(method, params, callType);
  }
  virtual void metaPost(AnyObject context, unsigned int event, const GenericFunctionParameters& params)
  {
    return _target.metaPost(event, params);
  }
  virtual qi::Future<SignalLink> metaConnect(unsigned int event, const SignalSubscriber& subscriber)
  {
    return _target.connect(event, subscriber);
  }
  /// Disconnect an event link. Returns if disconnection was successful.
  virtual qi::Future<void> metaDisconnect(SignalLink linkId)
  {
    return _target.disconnect(linkId);
  }
  virtual qi::Future<qi::AnyValue> metaProperty(unsigned int id)
  {
    return _target.property(id);
  }
  virtual qi::Future<void> metaSetProperty(unsigned int id, qi::AnyValue val)
  {
    return _target.setProperty(id, val);
  }
  qi::AnyObject _target;
};

static bool init()
{
  static bool ready = false;
  if (ready)
    return true;
  std::vector<std::string> res = qi::loadObject("cat");
  if (!res.size())
    return false;
  if (res.front() != "animals::Cat")
    return false;
  res = qi::loadObject("lynx");
  if (!res.size())
    return false;
  if (res.front() != "animals::Lynx")
    return false;
  ready = true;
  return true;
}


/* Very basic tests on service built on top of skeleton.
 * (Because we did not want complex patch procedures to inject
 * big chunks of code in it)
*/
TEST(TestCat, load)
{
  ASSERT_TRUE(init());
  qi::Object<animals::Cat> o = qi::createObject("animals::Cat");
  ASSERT_TRUE(!!o);
}

TEST(TestCat, any)
{
  ASSERT_TRUE(init());
  qi::AnyObject o = qi::createObject("animals::Cat");
  ASSERT_TRUE(!!o);
  std::cerr << o.metaObject().methodMap().size() << std::endl;
  qi::details::printMetaObject(std::cerr, o.metaObject(), true, true);
}

TEST(TestCat, proxy)
{
  ASSERT_TRUE(init());
  qi::Object<animals::Cat> o = qi::createObject("animals::Cat");
  ASSERT_TRUE(!!o);
  qi::AnyObject any = qi::makeDynamicAnyObject(new DummyBouncer(o));
  ASSERT_TRUE(!!any);
  qi::Object<animals::Cat> oproxy(any);
  ASSERT_TRUE(!!oproxy);
  animals::Mosquito m;
  animals::Cat& bareCat = oproxy.asT();
  ASSERT_TRUE(bareCat.setTarget(m));
  ASSERT_TRUE(oproxy->setTarget(m));
}


TEST(TestLynx, basic)
{
  ASSERT_TRUE(init());
  qi::Object<animals::Cat> o = qi::createObject("animals::Lynx");
  EXPECT_NO_THROW(o->meow(10));
  EXPECT_NO_THROW(o.call<void>("meow", 10));
  EXPECT_EQ(-2, o->boredom.get());
  EXPECT_EQ(-2, (float)o.property<float>("boredom"));
}

TEST(TestLynx, basicWrapped)
{
  ASSERT_TRUE(init());
  qi::Object<animals::Cat> ro = qi::createObject("animals::Lynx");
  ASSERT_TRUE(!!ro);
  qi::AnyObject any = qi::makeDynamicAnyObject(new DummyBouncer(ro));
  ASSERT_TRUE(!!any);
  qi::Object<animals::Cat> o(any);
  ASSERT_TRUE(!!o);
  EXPECT_NO_THROW(o->meow(10));
  EXPECT_NO_THROW(o.call<void>("meow", 10));
  EXPECT_EQ(-2, o->boredom.get());
  EXPECT_EQ(-2, (float)o.property<float>("boredom"));
  EXPECT_EQ(-2, (float)any.property<float>("boredom"));
  EXPECT_EQ(-2, (float)ro.property<float>("boredom"));
}

class CustomAction: public animals::CatAction
{
public:
  CustomAction() {}
  CustomAction(const std::string& n) : _name(n) {}
  std::string name() { return _name;}
  std::vector<float> expectedResult()
  { // expect carnage
    std::vector<float> res(_name.length(), 1);
    return res;
  }
  void run()
  {
    qi::os::msleep(_name.length()*100);
  }
  int useless; // just so we dont have LynxAction layout
  std::string _name;
};


/* obtain a temp object and pass it back to a method
* generate a temp object and pass it
*/
TEST(TestLynx, object)
{
  ASSERT_TRUE(init());
  qi::Object<animals::Cat> o = qi::createObject("animals::Lynx");
  qi::Object<CatAction> task = o->selectTask(0);
  ASSERT_TRUE(!!task);
  EXPECT_EQ("lynxYawn", task->name());
  ASSERT_TRUE(o->canPerform(task));
  EXPECT_ANY_THROW(o->canPerform(qi::Object<CatAction>()));
  task = new CustomAction("lynxDoABarrelRoll");
  ASSERT_TRUE(o->canPerform(task));
}

TEST(TestLynx, objectWrapped)
{
  ASSERT_TRUE(init());
  qi::Object<animals::Cat> ro = qi::createObject("animals::Lynx");
  ASSERT_TRUE(!!ro);
  qi::AnyObject any = qi::makeDynamicAnyObject(new DummyBouncer(ro));
  ASSERT_TRUE(!!any);
  qi::Object<animals::Cat> o(any);
  ASSERT_TRUE(!!o);
  qi::Object<CatAction> task = o->selectTask(0);
  ASSERT_TRUE(!!task);
  EXPECT_EQ("lynxYawn", task->name());
  ASSERT_TRUE(o->canPerform(task));
  EXPECT_ANY_THROW(o->canPerform(qi::Object<CatAction>()));
  task = new CustomAction("lynxDoABarrelRoll");
  ASSERT_TRUE(o->canPerform(task));
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
