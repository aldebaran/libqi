/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/



#include <map>
#include <gtest/gtest.h>
#include <qi/qi.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/session.hpp>

#include <qimessaging/service_directory.hpp>

qi::ServiceDirectory sd;
qi::Session          session1, session2;
qi::Object           oserver1, oserver2;
qi::Object          *oclient1, *oclient2;
static qi::Promise<bool> payload;
void onFire1(const int& pl)
{
  if (pl)
    oclient2->call<void>("onFire2", pl-1);
  else
    payload.setValue(true);
}

void onFire2(const int& pl)
{
  if (pl)
    oclient1->call<void>("onFire1", pl-1);
  else
    payload.setValue(true);
}

TEST(Test, Recurse)
{
  // Two objects with a fire event and a onFire method.
  ASSERT_TRUE(sd.listen("tcp://127.0.0.1:0"));
  ASSERT_TRUE(session1.connect(sd.listenUrl()));
  ASSERT_TRUE(session1.waitForConnected());
  ASSERT_TRUE(session2.connect(sd.listenUrl()));
  ASSERT_TRUE(session2.waitForConnected());
  oserver1.advertiseMethod("onFire1", &onFire1);
  oserver2.advertiseMethod("onFire2", &onFire2);
  ASSERT_TRUE(session1.listen("tcp://0.0.0.0:0"));
  ASSERT_TRUE(session2.listen("tcp://0.0.0.0:0"));
  ASSERT_GT(session1.registerService("coin1", &oserver1).wait(), 0);
  ASSERT_GT(session2.registerService("coin2", &oserver2).wait(), 0);
  EXPECT_EQ(1U, session1.registeredServices().size());
  EXPECT_EQ(1U, session2.registeredServices().size());
  oclient1 = session2.service("coin1");
  oclient2 = session1.service("coin2");
  int niter = 10000;
  if (getenv("VALGRIND"))
  {
    std::cerr << "Valgrind detected, reducing iteration count" << std::endl;
    niter = 50;
  }
  oclient1->call<void>("onFire1", niter);
  ASSERT_TRUE(payload.future().wait(3000));
  session1.close();
  session2.close();
  // We must force delete, otherwise destruction order is undefined.
  // And deleting a registered service is undefined behavior.
  session1.disconnect();
  session2.disconnect();
}
