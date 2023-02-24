/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>
#include <gtest/gtest.h>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/application.hpp>
#include <thread>
#include <chrono>

static qi::Atomic<int> lastPayload;
static qi::Atomic<int> lastPayload2;
static qi::Atomic<int> completed;

void onFire(const int& /*pl*/, qi::Promise<void>& p)
{
  ++lastPayload;
  p.setValue(0);
}
void onFire2(const int& /*pl*/, qi::Promise<void>& p)
{
  ++lastPayload2;
  p.setValue(0);
}

void testDelete(bool afirst, bool disconnectFirst, qi::Promise<void> end)
{
  qi::Promise<void> p0;
  qi::Promise<void> p1;
  unsigned int fireId, onFireId, onFireId2;
  qi::AnyObject *a;
  qi::AnyObject *b;
  {
    qi::DynamicObjectBuilder oba, obb;
    fireId = oba.advertiseSignal<int>("fire");
    onFireId = obb.advertiseMethod("onFire", boost::function<void(int)>(boost::bind<void>(&onFire, _1, boost::ref(p0))));
    onFireId2 = obb.advertiseMethod("onFire2", boost::function<void(int)>(boost::bind<void>(&onFire2, _1, boost::ref(p1))));
    a = new qi::AnyObject(oba.object());
    b = new qi::AnyObject(obb.object());
  }
  qi::SignalLink linkId = (*a).connect(fireId, *b, onFireId).value();
  ASSERT_TRUE(qi::isValidSignalLink(linkId));
  (*a).connect(fireId, *b, onFireId2);
  //std::vector<qi::SignalSubscriber> subs = (*a)->subscribers(fireId);
  //EXPECT_EQ(static_cast<unsigned int>(2), subs.size());
  // Subs ordering is unspecified
  //EXPECT_EQ(subs[0].method + subs[1].method, onFireId + onFireId2);
  lastPayload = 11;
  lastPayload2 = 11;
  (*a).post("fire", 12);
  EXPECT_TRUE(p0.future().hasValue(1000));
  EXPECT_TRUE(p1.future().hasValue(1000));
  EXPECT_EQ(12, lastPayload.load());
  EXPECT_EQ(12, lastPayload2.load());
  p0 = qi::Promise<void>();
  p1 = qi::Promise<void>();
  if (disconnectFirst)
  {
    (*a).disconnect(linkId);
    (*a).post("fire", 13);
    EXPECT_TRUE(p1.future().hasValue(1000));
    EXPECT_EQ(12, lastPayload.load());
    EXPECT_EQ(13, lastPayload2.load());
    p0 = qi::Promise<void>();
    p1 = qi::Promise<void>();
  }
  if (afirst)
  {
    delete a;
    delete b;
  }
  else
  {
    int e1 = lastPayload.load(), e2 = lastPayload2.load();
    delete b;
    // wait for the object to be deleted (signal callbacks may still be
    // running)
    std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
    (*a).post("fire", 12);
    EXPECT_EQ(e1, lastPayload.load());
    EXPECT_EQ(e2, lastPayload2.load());
    delete a;
  }
  ++completed;
  int next = completed.load();
  if (next == 4)
  {
    end.setValue(nullptr);
    return;
  }
  qi::getEventLoop()->post(
    boost::bind(&testDelete, !!((int)next/2), !!((int)next%2), end));
}

TEST(TestObject, Destruction)
{
  qi::Promise<void> end;
  completed = 0;
  qi::getEventLoop()->post(
    boost::bind(&testDelete, false, false, end));
  end.future().wait();
}
