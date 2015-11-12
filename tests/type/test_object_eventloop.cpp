/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

// Test that calls happen in the correct event loop

#include <boost/thread.hpp>

#include <gtest/gtest.h>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/objecttypebuilder.hpp>
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

void call_samethread(qi::AnyObject obj, qi::Promise<bool> res,
  void* tid)
{
  if (!tid)
    tid = new TID(boost::this_thread::get_id());
  res.setValue(obj.call<bool>("sameThread", (unsigned long)tid));
}

// Calls the sameThread method in givent event loop.
qi::Future<bool> callSameThreadIn(qi::AnyObject obj,
  qi::EventLoop* el, void* tid)
{
  qi::Promise<bool> p;
  el->post(boost::bind(call_samethread, obj, p, tid));
  return p.future();
}

void fire_samethread(qi::AnyObject obj, void* tid)
{
  if (!tid)
    tid = new TID(boost::this_thread::get_id());
  obj.post("fire", (unsigned long) tid);
}

// Fire sameThread in given event loop
void fireSameThreadIn(qi::AnyObject obj, qi::EventLoop* el, void* tid)
{
  el->post(boost::bind(fire_samethread, obj, tid));
}

qi::AnyObject makeDynamicObj()
{
  qi::DynamicObjectBuilder ob;
  ob.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  ob.advertiseMethod("sameThread", &sameThread);
  ob.advertiseSignal<unsigned long>("fire");
  qi::AnyObject res = ob.object();
  return res;
}

qi::AnyObject makeDynamicObjWithThreadModel(qi::ObjectThreadingModel model)
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("sameThread", &sameThread);
  ob.advertiseMethod("delayms", &qi::os::msleep);
  ob.advertiseMethod("delaymsThreadSafe", &qi::os::msleep, "", qi::MetaCallType_Queued);
  ob.advertiseMethod("delaymsFast", &qi::os::msleep, "", qi::MetaCallType_Direct);
  ob.advertiseSignal<unsigned long>("fire");
  ob.setThreadingModel(model);
  qi::AnyObject res = ob.object();
  return res;
}

class EventObject {
public:
  bool sameThread(const unsigned long& tid)
  {
    return ::sameThread(tid);
  }

  void delayms(unsigned int milliseconds)
  {
    qi::os::msleep(milliseconds);
  }

  qi::Signal<unsigned long> fire;

};

qi::AnyObject makeStaticObjWithThreadModel(EventObject &obj, qi::ObjectThreadingModel model)
{
  qi::ObjectTypeBuilder<EventObject>  ob;
  ob.advertiseMethod("sameThread", &EventObject::sameThread);
  ob.advertiseMethod("delayms", &EventObject::delayms);
  ob.advertiseMethod("delaymsThreadSafe", &EventObject::delayms, qi::MetaCallType_Queued);
  ob.advertiseMethod("delaymsFast", &EventObject::delayms, qi::MetaCallType_Direct);
  ob.advertiseSignal("fire", &EventObject::fire);
  ob.setThreadingModel(model);
  qi::AnyObject res = ob.object(&obj, &qi::AnyObject::deleteGenericObjectOnly);
  return res;
}

TEST(TestEventLoop, Basic)
{
  void* mainId = new TID(boost::this_thread::get_id());
  qi::AnyObject o1 = makeDynamicObj();
  // Call is synchronous, no reason not to
  ASSERT_TRUE(o1.call<bool>("sameThread", (unsigned long)mainId));
  // FIXME more!
}


TEST(TestThreadModel, notThreadSafe)
{
  new TID(boost::this_thread::get_id());
  qi::AnyObject o1 = makeDynamicObjWithThreadModel(qi::ObjectThreadingModel_SingleThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delayms", 150);
  o1.async<void>("delayms", 150).wait();
  f1.wait();
  // we expect >300ms result, take 10% marging to take into acount
  // timer granularity and sleep duration inprecision.
  ASSERT_GT(qi::os::ustime() - start, 270000);
}

TEST(TestThreadModel, ThreadSafe)
{
  new TID(boost::this_thread::get_id());
  qi::AnyObject o1 = makeDynamicObjWithThreadModel(qi::ObjectThreadingModel_MultiThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delaymsThreadSafe", 150);
  o1.async<void>("delaymsThreadSafe", 150).wait();
  f1.wait();
  ASSERT_LT(qi::os::ustime() - start, 270000);
}

TEST(TestThreadModel, MethodModel)
{
  qi::AnyObject o1 = makeDynamicObjWithThreadModel(qi::ObjectThreadingModel_SingleThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delaymsThreadSafe", 150);
  ASSERT_LT(qi::os::ustime() - start, 100000);
  f1.wait();
  start = qi::os::ustime();
  // fast method->synchronous call
  f1 = o1.async<void>("delaymsFast", 150);
  ASSERT_GT(qi::os::ustime() - start, 100000);
  ASSERT_TRUE(f1.isFinished());
  // Thread-safe method: parallel call
  start = qi::os::ustime();
  f1 = o1.async<void>("delaymsThreadSafe", 150);
  o1.async<void>("delaymsThreadSafe", 150).wait();
  f1.wait();
  ASSERT_LT(qi::os::ustime() - start, 270000);
}

TEST(TestThreadModelStatic, notThreadSafeObjectStatic)
{
  new TID(boost::this_thread::get_id());
  EventObject e;
  qi::AnyObject o1 = makeStaticObjWithThreadModel(e, qi::ObjectThreadingModel_SingleThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delayms", 150);
  o1.async<void>("delayms", 150).wait();
  f1.wait();
  // we expect >300ms result, take 10% marging to take into acount
  // timer granularity and sleep duration inprecision.
  ASSERT_GT(qi::os::ustime() - start, 270000);
}

TEST(TestThreadModelStatic, ThreadSafe)
{
  new TID(boost::this_thread::get_id());
  EventObject e;
  qi::AnyObject o1 = makeStaticObjWithThreadModel(e, qi::ObjectThreadingModel_MultiThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delaymsThreadSafe", 150);
  o1.async<void>("delaymsThreadSafe", 150).wait();
  f1.wait();
  ASSERT_LT(qi::os::ustime() - start, 270000);
}

TEST(TestThreadModelStatic, MethodModel)
{
  EventObject e;
  qi::AnyObject o1 = makeStaticObjWithThreadModel(e, qi::ObjectThreadingModel_SingleThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delaymsThreadSafe", 150);
  ASSERT_LT(qi::os::ustime() - start, 100000);
  f1.wait();
  start = qi::os::ustime();
  // fast method->synchronous call
  f1 = o1.async<void>("delaymsFast", 150);
  ASSERT_GT(qi::os::ustime() - start, 100000);
  ASSERT_TRUE(f1.isFinished());
  // Thread-safe method: parallel call
  start = qi::os::ustime();
  f1 = o1.async<void>("delaymsThreadSafe", 150);
  o1.async<void>("delaymsThreadSafe", 150).wait();
  f1.wait();
  ASSERT_LT(qi::os::ustime() - start, 270000);
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
