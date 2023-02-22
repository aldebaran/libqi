/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

// Test that calls happen in the correct event loop

#include <cstdint>

#include <boost/thread.hpp>

#include <gtest/gtest.h>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/application.hpp>


// ARGH! boost::thread::id is an opaque class, not an int, so we have
// to hack like hell to pass it in qimessaging calls: We pass along
// pointers to TID in an big-enough int type.

using TID = boost::thread::id;
using RawTID = std::uintptr_t;

bool sameThread(const RawTID& tid)
{
  boost::thread::id* id = reinterpret_cast<boost::thread::id*>(tid);
  bool res = *id == boost::this_thread::get_id();
  return res;
}

void fire_samethread(qi::AnyObject obj, void* tid)
{
  if (!tid)
    tid = new TID(boost::this_thread::get_id());
  obj.post("fire", reinterpret_cast<RawTID>(tid));
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
  ob.advertiseSignal<RawTID>("fire");
  qi::AnyObject res = ob.object();
  return res;
}

qi::AnyObject makeDynamicObjWithThreadModel(qi::ObjectThreadingModel model)
{
  static const auto sleep = [](qi::MilliSeconds dura){
    boost::this_thread::sleep_for(dura);
  };

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("sameThread", &sameThread);
  ob.advertiseMethod("delayms", sleep);
  ob.advertiseMethod("delaymsThreadSafe", sleep, "", qi::MetaCallType_Queued);
  ob.advertiseMethod("delaymsFast", sleep, "", qi::MetaCallType_Direct);
  ob.advertiseSignal<RawTID>("fire");
  ob.setThreadingModel(model);
  qi::AnyObject res = ob.object();
  return res;
}

class EventObject {
public:
  bool sameThread(const RawTID& tid)
  {
    return ::sameThread(tid);
  }

  void delayms(qi::MilliSeconds delay)
  {
    boost::this_thread::sleep_for(delay);
  }

  qi::Signal<RawTID> fire;

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
  auto mainId = boost::this_thread::get_id();
  qi::AnyObject o1 = makeDynamicObj();
  // Call is synchronous, no reason not to
  ASSERT_TRUE(o1.call<bool>("sameThread", reinterpret_cast<RawTID>(&mainId)));
  // FIXME more!
}


TEST(TestThreadModel, notThreadSafe)
{
  qi::AnyObject o1 = makeDynamicObjWithThreadModel(qi::ObjectThreadingModel_SingleThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delayms", qi::MilliSeconds{ 150 });
  o1.async<void>("delayms", qi::MilliSeconds{ 150 }).wait();
  f1.wait();
  // we expect >300ms result, take 10% marging to take into acount
  // timer granularity and sleep duration inprecision.
  ASSERT_GT(qi::os::ustime() - start, 270000);
}

TEST(TestThreadModel, ThreadSafe)
{
  qi::AnyObject o1 = makeDynamicObjWithThreadModel(qi::ObjectThreadingModel_MultiThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delaymsThreadSafe", qi::MilliSeconds{ 150 });
  o1.async<void>("delaymsThreadSafe", qi::MilliSeconds{ 150 }).wait();
  f1.wait();
  ASSERT_LT(qi::os::ustime() - start, 270000);
}

TEST(TestThreadModel, MethodModel)
{
  qi::AnyObject o1 = makeDynamicObjWithThreadModel(qi::ObjectThreadingModel_SingleThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delaymsThreadSafe", qi::MilliSeconds{ 150 });
  ASSERT_LT(qi::os::ustime() - start, 100000);
  f1.wait();
  start = qi::os::ustime();
  // fast method->synchronous call
  f1 = o1.async<void>("delaymsFast", qi::MilliSeconds{ 150 });
  ASSERT_GT(qi::os::ustime() - start, 100000);
  ASSERT_TRUE(f1.isFinished());
  // Thread-safe method: parallel call
  start = qi::os::ustime();
  f1 = o1.async<void>("delaymsThreadSafe", qi::MilliSeconds{ 150 });
  o1.async<void>("delaymsThreadSafe", qi::MilliSeconds{ 150 }).wait();
  f1.wait();
  ASSERT_LT(qi::os::ustime() - start, 270000);
}

TEST(TestThreadModelStatic, notThreadSafeObjectStatic)
{
  EventObject e;
  qi::AnyObject o1 = makeStaticObjWithThreadModel(e, qi::ObjectThreadingModel_SingleThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delayms", qi::MilliSeconds{ 150 });
  o1.async<void>("delayms", qi::MilliSeconds{ 150 }).wait();
  f1.wait();
  // we expect >300ms result, take 10% marging to take into acount
  // timer granularity and sleep duration inprecision.
  ASSERT_GT(qi::os::ustime() - start, 270000);
}

TEST(TestThreadModelStatic, ThreadSafe)
{
  EventObject e;
  qi::AnyObject o1 = makeStaticObjWithThreadModel(e, qi::ObjectThreadingModel_MultiThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delaymsThreadSafe", qi::MilliSeconds{ 150 });
  o1.async<void>("delaymsThreadSafe", qi::MilliSeconds{ 150 }).wait();
  f1.wait();
  ASSERT_LT(qi::os::ustime() - start, 270000);
}

TEST(TestThreadModelStatic, MethodModel)
{
  EventObject e;
  qi::AnyObject o1 = makeStaticObjWithThreadModel(e, qi::ObjectThreadingModel_SingleThread);
  qi::int64_t start = qi::os::ustime();
  qi::Future<void> f1 = o1.async<void>("delaymsThreadSafe", qi::MilliSeconds{ 150 });
  ASSERT_LT(qi::os::ustime() - start, 100000);
  f1.wait();
  start = qi::os::ustime();
  // fast method->synchronous call
  f1 = o1.async<void>("delaymsFast", qi::MilliSeconds{ 150 });
  ASSERT_GT(qi::os::ustime() - start, 100000);
  ASSERT_TRUE(f1.isFinished());
  // Thread-safe method: parallel call
  start = qi::os::ustime();
  f1 = o1.async<void>("delaymsThreadSafe", qi::MilliSeconds{ 150 });
  o1.async<void>("delaymsThreadSafe", qi::MilliSeconds{ 150 }).wait();
  f1.wait();
  ASSERT_LT(qi::os::ustime() - start, 270000);
}
