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
#include <qi/atomic.hpp>
#include <qimessaging/future.hpp>


class TestFuture : public ::testing::Test
{
public:
  static int         gGlobalI;
  static std::string gGlobalS;
  static std::string gGlobalE;
  static qi::atomic<long>   gSuccess;

protected:
  static void SetUpTestCase()
  {
    gGlobalI = 0;
    gGlobalS = "";
    gGlobalE = "";
    gSuccess = 0;
  }
};

int         TestFuture::gGlobalI;
std::string TestFuture::gGlobalS;
std::string TestFuture::gGlobalE;
qi::atomic<long> TestFuture::gSuccess;

class TestFutureI : public qi::FutureInterface<int> {
public:
  TestFutureI(int &gGlobalI, std::string &gGlobalE)
    : gGlobalI(gGlobalI),
      gGlobalE(gGlobalE)
  {}

  void onFutureFinished(const int &future, void *QI_UNUSED(data)) {
    gGlobalI = future;
  }

  void onFutureFailed(const std::string &error, void *QI_UNUSED(data)) {
    gGlobalE = error;
  }

  int         &gGlobalI;
  std::string &gGlobalE;
};

class TestFutureS : public qi::FutureInterface<std::string> {
public:
  TestFutureS(std::string &gGlobalS, std::string &gGlobalE)
    : gGlobalS(gGlobalS),
      gGlobalE(gGlobalE)
  {}

  void onFutureFinished(const std::string &future, void *QI_UNUSED(data)) {
    gGlobalS = future;
  }

  void onFutureFailed(const std::string &error, void *QI_UNUSED(data)) {
    gGlobalE = error;
  }

  std::string &gGlobalS;
  std::string &gGlobalE;
};

TEST_F(TestFuture, SimpleType) {
  TestFutureI tf(gGlobalI, gGlobalE);

  qi::Promise<int> pro;

  qi::Future<int>  fut = pro.future();

  fut.addCallbacks(&tf);

  EXPECT_EQ(0, gGlobalI);
  EXPECT_FALSE(fut.isReady());
  pro.setValue(42);
  EXPECT_TRUE(fut.isReady());
  EXPECT_EQ(42, fut.value());
  EXPECT_EQ(42, gGlobalI);
}

TEST_F(TestFuture, ComplexType) {
  TestFutureS tf(gGlobalS, gGlobalE);

  qi::Promise<std::string> pro;

  qi::Future<std::string>  fut = pro.future();

  fut.addCallbacks(&tf);

  EXPECT_STREQ("", gGlobalS.c_str());
  EXPECT_FALSE(fut.isReady());
  pro.setValue("42");
  EXPECT_TRUE(fut.isReady());
  EXPECT_STREQ("42", fut.value().c_str());
  EXPECT_STREQ("42", gGlobalS.c_str());
}

void producer(qi::Promise<int> pro) {
  qi::os::msleep(100);
  pro.setValue(42);
}

void consumer(qi::atomic<long> &gSuccess, qi::Future<int> fut) {
  //wont block thread on error
  ASSERT_TRUE(fut.wait(1000));
  EXPECT_EQ(42, fut.value());
  ++gSuccess;
}

TEST_F(TestFuture, Threaded) {
  qi::Promise<int> pro;
  EXPECT_EQ(0, *gSuccess);
  boost::thread_group tg;

  tg.create_thread(boost::bind(&consumer, boost::ref(gSuccess), pro.future()));
  tg.create_thread(boost::bind(&consumer, boost::ref(gSuccess), pro.future()));
  tg.create_thread(boost::bind(&consumer, boost::ref(gSuccess), pro.future()));
  tg.create_thread(boost::bind(&producer, pro));
  tg.join_all();
  EXPECT_EQ(3, *gSuccess);
}


TEST_F(TestFuture, TestTimeout) {
  qi::Promise<int> pro;
  qi::Future<int>  fut = pro.future();

  EXPECT_FALSE(fut.wait(100));
  EXPECT_FALSE(fut.isReady());
}

TEST_F(TestFuture, TestError) {
  TestFutureI tf(gGlobalI, gGlobalE);

  qi::Promise<int> pro;

  qi::Future<int>  fut = pro.future();
  fut.addCallbacks(&tf);

  EXPECT_STREQ("", gGlobalE.c_str());
  EXPECT_FALSE(fut.hasError());
  EXPECT_FALSE(fut.isReady());
  pro.setError("chiche");
  fut.wait();
  EXPECT_STREQ("chiche", gGlobalE.c_str());
  EXPECT_TRUE(fut.isReady());
  EXPECT_TRUE(fut.hasError());
}
