/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/



#include <map>
#include <gtest/gtest.h>
#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/genericobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/service_directory.hpp>

qi::GenericObject          oclient1, oclient2;
static qi::Promise<bool> payload;

void onFire1(const int& pl)
{
  if (pl)
    oclient2.call<void>("onFire2", pl-1).async();
  else
    payload.setValue(true);
}

void onFire2(const int& pl)
{
  if (pl)
    qi::Future<void> unused = oclient1.call<void>("onFire1", pl-1);
  else
    payload.setValue(true);
}

TEST(Test, Recurse)
{
  qi::ServiceDirectory  sd;
  qi::Session           session1, session2;
  qi::GenericObjectBuilder     ob1, ob2;
  ob1.advertiseMethod("onFire1", &onFire1);
  ob2.advertiseMethod("onFire2", &onFire2);
  qi::GenericObject    oserver1(ob1.object()), oserver2(ob2.object());

  // Two objects with a fire event and a onFire method.
  ASSERT_TRUE(sd.listen("tcp://127.0.0.1:0"));
  ASSERT_TRUE(session1.connect(sd.listenUrl()));
  ASSERT_TRUE(session2.connect(sd.listenUrl()));
  ASSERT_TRUE(session1.listen("tcp://0.0.0.0:0"));
  ASSERT_TRUE(session2.listen("tcp://0.0.0.0:0"));
  ASSERT_GT(session1.registerService("coin1", oserver1).wait(), 0);
  ASSERT_GT(session2.registerService("coin2", oserver2).wait(), 0);
  EXPECT_EQ(1U, session1.services(qi::Session::ServiceLocality_Local).value().size());
  EXPECT_EQ(1U, session2.services(qi::Session::ServiceLocality_Local).value().size());
  oclient1 = session2.service("coin1");
  oclient2 = session1.service("coin2");
#ifdef WIN32
  int niter = 1000;
#else
  int niter = 10000;
#endif
  if (getenv("VALGRIND"))
  {
    std::cerr << "Valgrind detected, reducing iteration count" << std::endl;
    niter = 50;
  }
  oclient1.call<void>("onFire1", niter);
  ASSERT_TRUE(payload.future().wait(3000));

  session1.close();
  session2.close();
  sd.close();
}


int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
