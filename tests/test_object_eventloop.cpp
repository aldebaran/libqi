/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

// Test that calls happen in the correct event loop

#include <boost/thread.hpp>

#include <gtest/gtest.h>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qi/application.hpp>


// ARGH! boost::thread::id is an opaque class, not an int, so we have
// to hack like hell to pass it in qimessaging calls: We pass along
// pointers to TID in an unsigned long.

typedef boost::thread::id TID;


bool sameThread(const unsigned long& tid)
{
  boost::thread::id* id = (boost::thread::id*)(void*)tid;
  bool res = *id == boost::this_thread::get_id();
  return res;
}

qi::Promise<bool> result;
void vSameThread(const unsigned long& tid)
{
  result.setValue(sameThread(tid));
}

void call_samethread(qi::ObjectPtr obj, qi::Promise<bool> res,
  void* tid)
{
  if (!tid)
    tid = new TID(boost::this_thread::get_id());
  res.setValue(obj->call<bool>("sameThread", (unsigned long)tid));
}

// Calls the sameThread method in givent event loop.
qi::Future<bool> callSameThreadIn(qi::ObjectPtr obj,
  qi::EventLoop* el, void* tid)
{
  qi::Promise<bool> p;
  el->post(boost::bind(call_samethread, obj, p, tid));
  return p.future();
}

void fire_samethread(qi::ObjectPtr obj, void* tid)
{
  if (!tid)
    tid = new TID(boost::this_thread::get_id());
  obj->emitEvent("fire", (unsigned long) tid);
}

// Fire sameThread in given event loop
void fireSameThreadIn(qi::ObjectPtr obj, qi::EventLoop* el, void* tid)
{
  el->post(boost::bind(fire_samethread, obj, tid));
}

qi::ObjectPtr makeObj()
{
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("sameThread", &sameThread);
  ob.advertiseEvent<void (unsigned long)>("fire");
  qi::ObjectPtr res = ob.object();
  return res;
}

qi::ObjectPtr makeObjWithThreadModel(qi::ObjectThreadingModel model)
{
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("sameThread", &sameThread);
  ob.advertiseMethod("delayms", &qi::os::msleep);
  ob.advertiseMethod("delaymsThreadSafe", &qi::os::msleep, "", qi::MetaCallType_Queued);
  ob.advertiseMethod("delaymsFast", &qi::os::msleep, "", qi::MetaCallType_Direct);
  ob.advertiseEvent<void (unsigned long)>("fire");
  ob.setThreadingModel(model);
  qi::ObjectPtr res = ob.object();
  return res;
}


TEST(TestEventLoop, Basic)
{
  void* mainId = new TID(boost::this_thread::get_id());
  qi::ObjectPtr o1 = makeObj();
  // Call is synchronous, no reason not to
  ASSERT_TRUE(o1->call<bool>("sameThread", (unsigned long)mainId));
  // FIXME more!
}

TEST(TestEventLoop, Event)
{
  unsigned long mainId = (unsigned long)(void*)new TID(boost::this_thread::get_id());
  qi::ObjectPtr o1 = makeObj();
  qi::Link link = o1->connect("fire", &vSameThread);
  o1->emitEvent("fire", mainId);
  ASSERT_TRUE(result.future().wait(3000));
  ASSERT_TRUE(result.future().value());
  result.reset();
  fireSameThreadIn(o1, qi::getDefaultObjectEventLoop(), 0);
  ASSERT_TRUE(result.future().wait(3000));
  ASSERT_TRUE(result.future().value());
  result.reset();
  o1->disconnect(link);
  link = o1->connect("fire", &vSameThread, qi::MetaCallType_Queued);
  fireSameThreadIn(o1, qi::getDefaultObjectEventLoop(), 0);
  ASSERT_TRUE(result.future().wait(3000));
  ASSERT_FALSE(result.future().value());
  result.reset();
}

TEST(TestThreadModel, notThreadSafe)
{
  new TID(boost::this_thread::get_id());
  qi::ObjectPtr o1 = makeObjWithThreadModel(qi::ObjectThreadingModel_SingleThread);
  ASSERT_TRUE(callSameThreadIn(o1, qi::getDefaultObjectEventLoop(),
    0));
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1->call<void>("delayms", 150);
  o1->call<void>("delayms", 150).wait();
  f1.wait();
  ASSERT_GT(qi::os::ustime() - start, 300000);
}

TEST(TestThreadModel, ThreadSafe)
{
  new TID(boost::this_thread::get_id());
  qi::ObjectPtr o1 = makeObjWithThreadModel(qi::ObjectThreadingModel_MultiThread);
  ASSERT_TRUE(callSameThreadIn(o1, qi::getDefaultObjectEventLoop(),
    0));
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1->call<void>("delaymsThreadSafe", 150);
  o1->call<void>("delaymsThreadSafe", 150).wait();
  f1.wait();
  ASSERT_LT(qi::os::ustime() - start, 300000);
}

TEST(TestThreadModel, MethodModel)
{
  qi::ObjectPtr o1 = makeObjWithThreadModel(qi::ObjectThreadingModel_SingleThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1->call<void>("delaymsThreadSafe", 150);
  ASSERT_LT(qi::os::ustime() - start, 100000);
  f1.wait();
  start = qi::os::ustime();
  // fast method->synchronous call
  f1 = o1->call<void>("delaymsFast", 150);
  ASSERT_GT(qi::os::ustime() - start, 100000);
  ASSERT_TRUE(f1.isReady());
  // Thread-safe method: parallel call
  start = qi::os::ustime();
  f1 = o1->call<void>("delaymsThreadSafe", 150);
  o1->call<void>("delaymsThreadSafe", 150).wait();
  f1.wait();
  ASSERT_LT(qi::os::ustime() - start, 300000);
}


int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
