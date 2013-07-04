/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/



#include <map>
#include <gtest/gtest.h>
#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsessionpair.hpp>

qi::AnyObject          oclient1, oclient2;
static qi::Promise<bool> payload;

void onFire1(const int& pl)
{
  if (pl)
    oclient2->call<void>("onFire2", pl-1).async();
  else
    payload.setValue(true);
}

void onFire2(const int& pl)
{
  if (pl)
    qi::Future<void> unused = oclient1->call<void>("onFire1", pl-1);
  else
    payload.setValue(true);
}

TEST(Test, Recurse)
{
  TestSessionPair       p1;
  TestSessionPair       p2(p1);
  qi::DynamicObjectBuilder     ob1, ob2;
  ob1.advertiseMethod("onFire1", &onFire1);
  ob2.advertiseMethod("onFire2", &onFire2);
  qi::AnyObject    oserver1(ob1.object()), oserver2(ob2.object());
  unsigned int           nbServices = TestMode::getTestMode() == TestMode::Mode_Nightmare ? 2 : 1;

  // Two objects with a fire event and a onFire method.
  ASSERT_NO_THROW(p1.server()->registerService("coin1", oserver1).hasValue(4000));
  ASSERT_NO_THROW(p2.server()->registerService("coin2", oserver2).hasValue(4000));
  EXPECT_EQ(nbServices, p1.server()->services(qi::Session::ServiceLocality_Local).value().size());
  EXPECT_EQ(nbServices, p2.server()->services(qi::Session::ServiceLocality_Local).value().size());
  oclient1 = p2.client()->service("coin1");
  oclient2 = p1.client()->service("coin2");
#ifdef WIN32
  int niter = 1000;
#else
  int niter = 10000;
#endif
  if (TestMode::getTestMode() == TestMode::Mode_SSL)
  {
    niter /= 100;
  }
  if (getenv("VALGRIND"))
  {
    std::cerr << "Valgrind detected, reducing iteration count" << std::endl;
    niter = 50;
  }
  oclient1->call<void>("onFire1", niter);
  ASSERT_NO_THROW(payload.future().hasValue(15000));
  oclient1.reset();
  oclient2.reset();
}


int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
