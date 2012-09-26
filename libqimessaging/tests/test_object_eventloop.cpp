/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

// Test that calls happen in the correct event loop

#include <boost/thread.hpp>

#include <gtest/gtest.h>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/genericobjectbuilder.hpp>
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
  el->asyncCall(0,
    boost::bind(call_samethread, obj, p, tid));
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
  el->asyncCall(0, boost::bind(fire_samethread, obj, tid));
}

qi::ObjectPtr makeObj(qi::EventLoop* el  = qi::getDefaultObjectEventLoop())
{
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("sameThread", &sameThread);
  ob.advertiseEvent<void (unsigned long)>("fire");
  qi::ObjectPtr res = ob.object();
  res->moveToEventLoop(el);
  return res;
}

TEST(TestEventLoop, Basic)
{
  void* mainId = new TID(boost::this_thread::get_id());
  qi::ObjectPtr o1 = makeObj();
  ASSERT_FALSE(o1->call<bool>("sameThread", (unsigned long)mainId));
  ASSERT_FALSE(callSameThreadIn(o1, qi::getDefaultNetworkEventLoop(),
    mainId));
  ASSERT_FALSE(callSameThreadIn(o1, qi::getDefaultNetworkEventLoop(),
    0));
  ASSERT_TRUE(callSameThreadIn(o1, qi::getDefaultObjectEventLoop(),
    0));
}

TEST(TestEventLoop, Event)
{
  unsigned long mainId = (unsigned long)(void*)new TID(boost::this_thread::get_id());
  qi::ObjectPtr o1 = makeObj();
  o1->connect("fire", &vSameThread);
  o1->emitEvent("fire", mainId);
  ASSERT_TRUE(result.future().wait(3000));
  ASSERT_FALSE(result.future().value());
  result.reset();
  fireSameThreadIn(o1, qi::getDefaultObjectEventLoop(), 0);
  ASSERT_TRUE(result.future().wait(3000));
  ASSERT_TRUE(result.future().value());
  result.reset();
  fireSameThreadIn(o1, qi::getDefaultNetworkEventLoop(), 0);
  ASSERT_TRUE(result.future().wait(3000));
  ASSERT_FALSE(result.future().value());
  result.reset();
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
