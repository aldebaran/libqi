/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <map>
#include <string>
#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <qi/os.hpp>
#include <qimessaging/future.hpp>

static int         gGlobalI(0);
static std::string gGlobalS("");
static std::string gGlobalE("");

class TestFutureI : public qi::FutureInterface<int> {
public:
  void onFutureFinished(const qi::Future<int> &future, void *data) {
    gGlobalI = future.value();
  }

  void onFutureFailed(const qi::Future<int> &future, void *data) {
    gGlobalE = future.error();
  }
};

class TestFutureS : public qi::FutureInterface<std::string> {
public:
  void onFutureFinished(const qi::Future<std::string> &future, void *data) {
    gGlobalS = future.value();
  }

  void onFutureFailed(const qi::Future<std::string> &future, void *data) {
    gGlobalE = future.error();
  }
};

TEST(TestFuture, SimpleType) {
  TestFutureI tf;

  qi::Promise<int> pro;

  qi::Future<int>  fut = pro.future();

  fut.setCallback(&tf);

  EXPECT_EQ(0, gGlobalI);
  EXPECT_FALSE(fut.isReady());
  pro.setValue(42);
  EXPECT_TRUE(fut.isReady());
  EXPECT_EQ(42, fut.value());
  EXPECT_EQ(42, gGlobalI);
}

TEST(TestFuture, ComplexType) {
  TestFutureS tf;

  qi::Promise<std::string> pro;

  qi::Future<std::string>  fut = pro.future();

  fut.setCallback(&tf);

  EXPECT_STREQ("", gGlobalS.c_str());
  EXPECT_FALSE(fut.isReady());
  pro.setValue("42");
  EXPECT_TRUE(fut.isReady());
  EXPECT_STREQ("42", fut.value().c_str());
  EXPECT_STREQ("42", gGlobalS.c_str());
}

static int gSuccess = 0;
void producer(qi::Promise<int> pro) {
  qi::os::msleep(100);
  pro.setValue(42);
}

void consumer(qi::Future<int> fut) {
  //wont block thread on error
  ASSERT_TRUE(fut.wait(1000));
  EXPECT_EQ(42, fut.value());
  gSuccess++;
}

TEST(TestFuture, Threaded) {
  qi::Promise<int> pro;
  EXPECT_EQ(0, gSuccess);
  boost::thread_group tg;

  tg.create_thread(boost::bind(&consumer, pro.future()));
  tg.create_thread(boost::bind(&consumer, pro.future()));
  tg.create_thread(boost::bind(&consumer, pro.future()));
  tg.create_thread(boost::bind(&producer, pro));
  tg.join_all();
  EXPECT_EQ(3, gSuccess);
}


TEST(TestFuture, TestTimeout) {
  qi::Promise<int> pro;
  qi::Future<int>  fut = pro.future();

  EXPECT_FALSE(fut.wait(100));
  EXPECT_FALSE(fut.isReady());
}

TEST(TestFuture, TestError) {
  TestFutureI tf;

  qi::Promise<int> pro;

  qi::Future<int>  fut = pro.future();
  fut.setCallback(&tf);

  EXPECT_STREQ("", gGlobalE.c_str());
  EXPECT_FALSE(fut.hasError());
  EXPECT_FALSE(fut.isReady());
  pro.setError("chiche");
  fut.wait();
  EXPECT_STREQ("chiche", gGlobalE.c_str());
  EXPECT_TRUE(fut.isReady());
  EXPECT_TRUE(fut.hasError());
}
