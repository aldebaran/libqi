/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/



#include <map>
#include <gtest/gtest.h>
#include <qimessaging/object.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/server.hpp>
#include <qimessaging/service_directory.hpp>

static int lastPayload = 0;
static unsigned int msDelay = 200;

void onFire(const int& pl)
{
  lastPayload = pl;
}

qi::Object        oserver;
static void setupServer(const std::string& sd)
{
  qi::Session&       session = *new qi::Session();
  qi::Server&        srv = *new qi::Server();
  oserver.advertiseEvent<void (*)(int)>("fire");
  session.connect(sd);
  session.waitForConnected();
  srv.listen(&session, "tcp://0.0.0.0:0");
  srv.registerService("coin", &oserver);
  EXPECT_EQ(1U, srv.registeredServices().size());
}
TEST(TestObject, Simple)
{
  qi::ServiceDirectory sd;
  unsigned int sdPort = qi::os::findAvailablePort(5555);
  std::stringstream sdAddr;
  sdAddr << "tcp://127.0.0.1:" << sdPort;
  sd.listen(sdAddr.str());
  setupServer(sdAddr.str());
  qi::Session client;
  client.connect(sdAddr.str());
  client.waitForConnected();
  std::vector<qi::ServiceInfo> services = client.services();
    EXPECT_EQ(2U, services.size());
  qi::Object* oclient = client.service("coin");

  for (unsigned i=0; i<5; ++i)
  {
    int linkId = oclient->connect("fire", &onFire);
    qi::os::msleep(msDelay);
    oserver.emitEvent("fire", 42);
    qi::os::msleep(msDelay);
    EXPECT_EQ(42, lastPayload);
    oserver.emitEvent("fire", 51);
    qi::os::msleep(msDelay);
    EXPECT_EQ(51, lastPayload);
    oclient->disconnect(linkId);
    oserver.emitEvent("fire", 42);
    qi::os::msleep(msDelay);
    EXPECT_EQ(51, lastPayload);
  }
}
