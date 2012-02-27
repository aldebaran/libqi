/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <map>
#include <gtest/gtest.h>
#include <qimessaging/future.hpp>

static int gGlobal = 0;

class TestFuture : public qi::FutureInterface<int> {
public:
  void onFutureFinished(const qi::Future<int> *future) {
    gGlobal = future->value();
  }
};
TEST(TestFuture, Simple) {
  TestFuture tf;

  qi::Promise<int> pro;

  qi::Future<int> *fut = pro.future();

  fut->callback(&tf);

  EXPECT_EQ(0, gGlobal);
  EXPECT_FALSE(fut->isReady());
  pro.setValue(42);
  EXPECT_TRUE(fut->isReady());
  EXPECT_EQ(42, fut->value());
  EXPECT_EQ(42, gGlobal);
}
