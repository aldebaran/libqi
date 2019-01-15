/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <map>
#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <qi/testutils/testutils.hpp>
#include <gtest/gtest.h>

qi::AnyObject oclient1, oclient2;
static qi::Promise<bool> payload;

void onFire1(const int& pl)
{
  if (pl)
    oclient2.async<void>("onFire2", pl-1);
  else
    payload.setValue(true);
}

void onFire2(const int& pl)
{
  if (pl)
    oclient1.async<void>("onFire1", pl-1);
  else
    payload.setValue(true);
}

TEST(Test, Recurse)
{
  payload = qi::Promise<bool>();

  TestSessionPair       p1;
  TestSessionPair       p2(TestSessionPair::ShareServiceDirectory, p1);
  qi::DynamicObjectBuilder     ob1, ob2;
  ob1.advertiseMethod("onFire1", &onFire1);
  ob2.advertiseMethod("onFire2", &onFire2);
  qi::AnyObject    oserver1(ob1.object()), oserver2(ob2.object());
  unsigned int           nbServices = TestMode::getTestMode() == TestMode::Mode_Nightmare ? 2 : 1;

  // Two objects with a fire event and a onFire method.

  ASSERT_TRUE(test::finishesWithValue(p1.server()->registerService("coin1", oserver1)));
  ASSERT_TRUE(test::finishesWithValue(p2.server()->registerService("coin2", oserver2)));
  EXPECT_EQ(nbServices, p1.server()->services(qi::Session::ServiceLocality_Local).value().size());
  EXPECT_EQ(nbServices, p2.server()->services(qi::Session::ServiceLocality_Local).value().size());

  ASSERT_TRUE(test::finishesWithValue(p2.client()->waitForService("coin1")));
  ASSERT_TRUE(test::finishesWithValue(p1.client()->waitForService("coin2")));
  oclient1 = p2.client()->service("coin1").value();
  oclient2 = p1.client()->service("coin2").value();
  int niter = 1000;
  if (TestMode::getTestMode() == TestMode::Mode_SSL)
  {
    niter /= 100;
  }
  if (getenv("VALGRIND"))
  {
    std::cerr << "Valgrind detected, reducing iteration count" << std::endl;
    niter = 50;
  }
  oclient1.call<void>("onFire1", niter);
  ASSERT_TRUE(test::finishesWithValue(payload.future()));
  oclient1.reset();
  oclient2.reset();
}
