/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <gtest/gtest.h>
#include <qimessaging/object.hpp>

static int gGlobalResult = 0;

void vfun(const int &p0,const int &p1)   { gGlobalResult = p0 + p1; }
int fun(const int &p0,const int &p1)     { return p0 + p1; }

struct Foo {
  int fun(const int &p0,const int &p1)   { return p0 + p1; }
  void vfun(const int &p0,const int &p1) { gGlobalResult = p0 + p1; }
};


TEST(TestObject, Simple) {
  qi::Object obj;
  Foo        foo;

  obj.advertiseMethod("test", &fun);
  obj.advertiseMethod("vtest", &vfun);
  obj.advertiseMethod("objtest", &foo, &Foo::fun);
  obj.advertiseMethod("objvtest", &foo, &Foo::vfun);
  obj.advertiseMethod("testBind", (boost::function<int(int)>)boost::bind(&fun, 21, _1));

  EXPECT_EQ(42, obj.call<int>("test", 21, 21));
  EXPECT_EQ(42, obj.call<int>("objtest", 21, 21));
  EXPECT_EQ(42, obj.call<int>("testBind", 21));

  gGlobalResult = 0;
  obj.call<void>("vtest", 21, 21);
  EXPECT_EQ(42, gGlobalResult);
  gGlobalResult = 0;
  obj.call<void>("objvtest", 21, 21);
  EXPECT_EQ(42, gGlobalResult);
}
