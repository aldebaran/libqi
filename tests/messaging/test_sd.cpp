/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <qi/session.hpp>
#include <qi/testutils/testutils.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>

extern std::string simpleSdPath;
extern std::string mirrorSdPath;

qiLogCategory("TestSD");

TEST(ServiceDirectory, DoubleListen)
{
  auto sd = qi::makeSession();

  try {
    sd->listenStandalone("tcp://127.0.0.1:0");
    sd->listen("tcp://127.0.0.1:0");
  }
  catch(std::runtime_error& e)
  {
    qiLogError("test_sd") << e.what();
    ASSERT_TRUE(false);
  }
}

struct Serv
{
  static const int response;
  int f()
  {
    return response;
  }
};
const int Serv::response = 4242;

QI_REGISTER_OBJECT(Serv, f);

TEST(ServiceDirectory, MultiRegister)
{
  auto sd1 = qi::makeSession();
  auto sd2 = qi::makeSession();

  sd1->listenStandalone("tcp://127.0.0.1:0");
  sd2->listenStandalone("tcp://127.0.0.1:0");

  sd1->registerService("Serv", boost::make_shared<Serv>());
  sd2->registerService("Serv", boost::make_shared<Serv>());

  {
    auto client = qi::makeSession();
    client->connect(sd1->url());
    ASSERT_EQ(Serv::response, client->service("Serv").value().call<int>("f"));
  }
  {
    auto client = qi::makeSession();
    client->connect(sd2->url());
    ASSERT_EQ(Serv::response, client->service("Serv").value().call<int>("f"));
  }
}

TEST(ServiceDirectory, Republish)
{
  auto sd1 = qi::makeSession();
  auto sd2 = qi::makeSession();

  sd1->listenStandalone("tcp://127.0.0.1:0");
  sd2->listenStandalone("tcp://127.0.0.1:0");

  sd1->registerService("Serv", boost::make_shared<Serv>());

  sd2->registerService("Serv", sd1->service("Serv").value());

  ASSERT_EQ(Serv::response, sd1->service("Serv").value().call<int>("f"));
  ASSERT_EQ(Serv::response, sd2->service("Serv").value().call<int>("f"));
  {
    auto client = qi::makeSession();
    client->connect(sd1->url());
    ASSERT_EQ(Serv::response, client->service("Serv").value().call<int>("f"));
  }
  {
    auto client = qi::makeSession();
    client->connect(sd2->url());
    ASSERT_EQ(Serv::response, client->service("Serv").value().call<int>("f"));
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
  ASSERT_EQ(Serv::response, remoteRemoteService.call<int>("f"));
}

TEST(ServiceDirectory, RegisterServiceFromNonListeningSessionAndCallThroughAnIntermediate)
{
  auto sessionMainServer = qi::makeSession();
  auto sessionMainClient = qi::makeSession();
  auto sessionSecondaryFromMain = qi::makeSession();
  auto sessionSecondaryServer = qi::makeSession();
  auto sessionSecondaryClient = qi::makeSession();

  sessionMainServer->listenStandalone("tcp://127.0.0.1:0");
  sessionSecondaryServer->listenStandalone("tcp://127.0.0.1:0");
  sessionMainClient->connect(sessionMainServer->endpoints()[0]);
  sessionSecondaryFromMain->connect(sessionMainServer->endpoints()[0]);
  sessionSecondaryClient->connect(sessionSecondaryServer->endpoints()[0]);

  // Dummy services are registered to offset the indexes, so that they
  // cannot match by chance between the two service directories
  sessionMainServer->registerService("A", boost::make_shared<Serv>());
  sessionMainServer->registerService("B", boost::make_shared<Serv>());
  sessionMainServer->registerService("C", boost::make_shared<Serv>());

  auto originalService = boost::make_shared<Serv>();
  sessionMainClient->listen("tcp://localhost:0");
  sessionMainClient->registerService("Service", originalService);

  auto remoteService = sessionSecondaryFromMain->service("Service").value();
  sessionSecondaryServer->registerService("Service", remoteService);

  auto remoteRemoteService = sessionSecondaryClient->service("Service").value();
  ASSERT_EQ(Serv::response, remoteRemoteService.call<int>("f"));
}

TEST(ServiceDirectory, MirrorServicesBetweenProcesses)
{
  static const qi::MilliSeconds sleepDuration{ 50 };
  using test::ScopedProcess;
  ScopedProcess mainSd{simpleSdPath, {"--qi-listen-url=tcp://127.0.0.1:54321", "--qi-standalone"}};
  auto mainClient = qi::makeSession();
  for (int i = 0; i < 20; ++i)
  {
    boost::this_thread::sleep_for(sleepDuration);
    try
    {
      mainClient->connect("tcp://127.0.0.1:54321");
      break;
    }
    catch (const std::exception& e)
    {
      qiLogInfo() << "Main Service Directory is not ready yet (" << e.what() << ")";
    }
  }
  ASSERT_TRUE(mainClient->isConnected());
  mainClient->listen("tcp://localhost:0");
  mainClient->registerService("Service", boost::make_shared<Serv>());

  auto secondaryClient = qi::makeSession();
  ScopedProcess secondarySd{mirrorSdPath, {"--qi-url=tcp://127.0.0.1:54321", "--qi-listen-url=tcp://127.0.0.1:65432"}};
  for (int i = 0; i < 20; ++i)
  {
    boost::this_thread::sleep_for(sleepDuration);
    try
    {
      secondaryClient->connect("tcp://127.0.0.1:65432");
      break;
    }
    catch (const std::exception& e)
    {
      qiLogInfo() << "Secondary Service Directory is not ready yet (" << e.what() << ")";
    }
  }
  secondaryClient->waitForService("Service").value(2000);
  auto service = secondaryClient->service("Service").value(2000);
  ASSERT_EQ(Serv::response, service.call<int>("f"));
}

class PromisedObject
{
public:
  PromisedObject(qi::Promise<void> prom) : p(prom), a(0) { }
  ~PromisedObject()
  {
      p.setValue(0);
  }
  qi::Promise<void> p;
  int a;
};
QI_REGISTER_OBJECT(PromisedObject, a)

TEST(ServiceDirectory, NoThreadSpawnOnClientClose)
{
  const int objectCount = 200;
  qi::getEventLoop()->setMaxThreads(boost::thread::hardware_concurrency() + 1);

  auto originalServer = qi::makeSession();
  auto originalClient = qi::makeSession();
  auto mirroredServer = qi::makeSession();
  auto mirroredClient = qi::makeSession();

  originalServer->listenStandalone("tcp://127.0.0.1:0");
  originalClient->connect(originalServer->endpoints().back());
  mirroredServer->listenStandalone("tcp://127.0.0.1:0");
  mirroredClient->connect(mirroredServer->endpoints().back());

  const std::string serviceName{ "Cookies" };
  const std::string methodName{ "muffins" };
  std::vector<qi::Future<void>> futures;
  qi::DynamicObjectBuilder builder;
  builder.advertiseMethod(methodName, [&] {
    std::vector<qi::AnyObject> objects;
    for (int i = 0; i < objectCount; ++i)
    {
      qi::Promise<void> promise;
      objects.push_back(boost::make_shared<PromisedObject>(promise));
      futures.push_back(promise.future());
    }
    return objects;
  });
  originalServer->registerService(serviceName, builder.object());

  const auto mirrored = originalClient->service(serviceName).value();
  mirroredServer->registerService(serviceName, mirrored);

  const auto objects = mirroredClient->service(serviceName).value().call<std::vector<qi::AnyObject>>(methodName);

  qiLogVerbose() << "Closing client";
  mirroredClient->close().async();

  qiLogVerbose() << "Waiting for objects destruction";
  for (auto& future : futures)
  {
    ASSERT_TRUE(test::finishesWithValue(future, test::detail::DoNothing{}));
  }
}

TEST(ServiceDirectory, IsNotConnectedAfterClose)
{
  auto session = qi::makeSession();
  session->listenStandalone("tcp://127.0.0.1:0");
  session->close();
  ASSERT_FALSE(session->isConnected());
}
