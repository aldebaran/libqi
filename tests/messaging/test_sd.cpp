/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <qi/session.hpp>

TEST(ServiceDirectory, DoubleListen)
{
  qi::Session sd;

  try {
    sd.listenStandalone("tcp://127.0.0.1:0");
    sd.listen("tcp://127.0.0.1:0");
  }
  catch(std::runtime_error& e)
  {
    qiLogError("test_sd") << e.what();
    ASSERT_TRUE(false);
  }
}

struct Serv
{
  int f()
  {
    return 4242;
  }
};
QI_REGISTER_OBJECT(Serv, f);

TEST(ServiceDirectory, MultiRegister)
{
  qi::Session sd1;
  qi::Session sd2;

  sd1.listenStandalone("tcp://127.0.0.1:0");
  sd2.listenStandalone("tcp://127.0.0.1:0");

  sd1.registerService("Serv", boost::make_shared<Serv>());
  sd2.registerService("Serv", boost::make_shared<Serv>());

  {
    qi::Session client;
    client.connect(sd1.url());
    ASSERT_EQ(4242, client.service("Serv").value().call<int>("f"));
  }
  {
    qi::Session client;
    client.connect(sd2.url());
    ASSERT_EQ(4242, client.service("Serv").value().call<int>("f"));
  }
}

TEST(ServiceDirectory, Republish)
{
  qi::Session sd1;
  qi::Session sd2;

  sd1.listenStandalone("tcp://127.0.0.1:0");
  sd2.listenStandalone("tcp://127.0.0.1:0");

  sd1.registerService("Serv", boost::make_shared<Serv>());

  sd2.registerService("Serv", sd1.service("Serv"));

  ASSERT_EQ(4242, sd1.service("Serv").value().call<int>("f"));
  ASSERT_EQ(4242, sd2.service("Serv").value().call<int>("f"));
  {
    qi::Session client;
    client.connect(sd1.url());
    ASSERT_EQ(4242, client.service("Serv").value().call<int>("f"));
  }
  {
    qi::Session client;
    client.connect(sd2.url());
    ASSERT_EQ(4242, client.service("Serv").value().call<int>("f"));
  }
}

TEST(ServiceDirectory, ServiceEndpointMatchesServiceDirectorys)
{
  auto session = qi::makeSession();
  session->listenStandalone("tcp://127.0.0.1:0");
  session->registerService("Service", boost::make_shared<Serv>());
  auto serviceInfo = session->services().value().back();
  ASSERT_EQ(session->endpoints(), serviceInfo.endpoints());
}

TEST(ServiceDirectory, ReRegisterRemoteServiceRenewEndpoints)
{
  auto session1 = qi::makeSession();
  auto session2 = qi::makeSession();
  session1->listenStandalone("tcp://127.0.0.1:0");
  session2->listenStandalone("tcp://127.0.0.1:0");
  session1->registerService("Service", boost::make_shared<Serv>());
  auto serviceInfo1 = session1->services().value().back();
  auto remoteService = session1->service("Service").value();
  session2->registerService("Service", remoteService);
  auto serviceInfo2 = session2->services().value().back();
  ASSERT_NE(serviceInfo1.endpoints(), serviceInfo2.endpoints());
  ASSERT_NE(session1->endpoints(), serviceInfo2.endpoints());
  ASSERT_EQ(session2->endpoints(), serviceInfo2.endpoints());
}

TEST(ServiceDirectory, CallMessagesAreProperlyDispatched)
{
  auto session1 = qi::makeSession();
  auto session2 = qi::makeSession();
  session1->listenStandalone("tcp://127.0.0.1:0");
  session2->listenStandalone("tcp://127.0.0.1:0");

  // Dummy services are registered to offset the indexes, so that they
  // cannot match by chance between the two service directories
  auto originalService = boost::make_shared<Serv>();
  session2->registerService("A", boost::make_shared<Serv>());
  session2->registerService("B", boost::make_shared<Serv>());
  session2->registerService("C", boost::make_shared<Serv>());
  session1->registerService("Service", originalService);

  auto remoteService = session1->service("Service").value();
  session2->registerService("Service", remoteService);

  auto remoteRemoteService = session2->service("Service").value();
  ASSERT_EQ(4242, remoteRemoteService.call<int>("f"));
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

