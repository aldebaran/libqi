/*
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <vector>
#include <string>
#include <future>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

#include <qi/session.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <qi/signalspy.hpp>
#include <qi/testutils/testutils.hpp>

#include <testsession/testsessionpair.hpp>
#include "objectio.hpp"

using namespace qi;
using namespace test;

qiLogCategory("qi.test_session");

namespace
{
  static const auto defaultWaitDisconnectedSignalDuration = std::chrono::milliseconds{ 200 };
  static const auto defaultWaitLoopDuration = std::chrono::microseconds{ 50 };
  static const auto defaultWaitServiceDuration = qi::MilliSeconds{ 500 };
  static const auto dummyServiceName = "serviceTest";
  static const auto invalidUrl = "invalid url";

  struct SessionSequencer : Trackable<SessionSequencer>
  {
    ExecutionContext& execCtx;
    Future<void> startFuture;
    Future<void> stopFuture;
    std::atomic<int> count;

    SessionSequencer(ExecutionContext& execCtx,
                     Future<void> startFuture = Future<void>{ nullptr },
                     Future<void> stopFuture = Future<void>{ nullptr })
      : execCtx(execCtx)
      , startFuture{ startFuture }
      , stopFuture{ stopFuture }
      , count{ 0 }
    {
    }

    ~SessionSequencer()
    {
      Trackable<SessionSequencer>::destroy();
    }

    Future<void> operator()(bool bare)
    {
      // All calls to the same object will wait for the same future to be set to create a session
      // and make it listen, and then will wait again for another future to be set to stop it.
      // This way it is more likely that calls to session creation and closing will really be
      // concurrent.
      auto startAndCloseSession = track([=] {
        auto s = makeSession();
        if (!bare)
          s->listen("tcp://localhost:0");
        ++count;
        return stopFuture
            .andThen(FutureCallbackType_Sync,
                     [=](void*) { return execCtx.async([=]() { s->close(); }); })
            .unwrap();
      }, this);

      return startFuture
          .andThen(
              FutureCallbackType_Sync,
              track([=](void*) mutable { return execCtx.async(startAndCloseSession).unwrap(); },
                    this))
          .unwrap();
    }
  };

  std::string reply(const std::string& msg)
  {
    return msg;
  }

  AnyObject dummyDynamicObject()
  {
    DynamicObjectBuilder ob;
    ob.advertiseMethod("reply", &reply);
    return ob.object();
  }
}

namespace std
{
  std::ostream& operator<<(std::ostream& os, const std::vector<ServiceInfo>& services)
  {
    bool first = true;
    for (const auto& service : services)
    {
      if (!first)
      {
        os << ", ";
      }
      os << service.name();
      first = false;
    }
    return os;
  }
}

// KEEP ME FIRST
TEST(TestSession, CreateMany)
{
  // avoid spawning threads in qi::Application event loop by using our own
  EventLoop threadPool {"testsession_createmany_eventloop", 11, false};

  Promise<void> startPromise;
  Promise<void> stopPromise;

  // A lot of static init is going on, check that it is thread safe
  const auto sequenceSession =
      std::make_shared<SessionSequencer>(threadPool, startPromise.future(), stopPromise.future());
  const Future<void> futResults[] {
    (*sequenceSession)(true),
    (*sequenceSession)(true),
    (*sequenceSession)(true),
    (*sequenceSession)(true),
    (*sequenceSession)(false),
    (*sequenceSession)(false),
    (*sequenceSession)(false),
    (*sequenceSession)(false),
    (*sequenceSession)(false),
    (*sequenceSession)(true),
    (*sequenceSession)(false),
    (*sequenceSession)(true),
  };

  startPromise.setValue(nullptr);
  while (sequenceSession->count != 12)
  {
    std::this_thread::sleep_for(defaultWaitLoopDuration);
  }
  stopPromise.setValue(nullptr);
  for (auto& futResult : futResults)
  {
    futResult.wait();
  }
}

TEST(TestSession, CreateOne)
{
  auto session = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(session->listenStandalone(test::defaultListenUrl())));
  ASSERT_FALSE(session->endpoints().empty());
}

TEST(TestSession, TrivialDirectConnection)
{
  auto session1 = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(session1->listenStandalone(test::defaultListenUrl())));
  ASSERT_FALSE(session1->endpoints().empty());

  auto session2 = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(session2->connect(test::url(*session1))));
  ASSERT_TRUE(session2->isConnected());
}

namespace
{
  Future<void> synchronizedClose(Future<void> closeStartFut, const SessionPtr& s)
  {
    const auto weakSession = ka::weak_ptr(s);
    return closeStartFut.andThen([=](void*){
      if (auto sharedSession = weakSession.lock())
        return sharedSession->close().async();
      return makeFutureError<void>("Session has been destroyed.");
    }).unwrap();
  }

}

TEST(TestSession, MultiClose)
{
  TestSessionPair sessionPair;
  const auto& serverPtr = sessionPair.server();
  const auto& clientPtr = sessionPair.client();

  Promise<void> closeStartProm;
  auto closeStartFut = closeStartProm.future();

  const Future<void> futResults[] {
    synchronizedClose(closeStartFut, clientPtr),
    synchronizedClose(closeStartFut, clientPtr),
    synchronizedClose(closeStartFut, clientPtr),
    synchronizedClose(closeStartFut, serverPtr),
    synchronizedClose(closeStartFut, serverPtr),
    synchronizedClose(closeStartFut, serverPtr),
  };
  closeStartProm.setValue(nullptr);

  for (auto& futResult : futResults)
  {
    ASSERT_TRUE(finishesWithValue(futResult)); // close succeeds even when already closed
  }
}

TEST(TestSession, SimpleConnectionToSd)
{
  // TODO: This is more a test of TestSessionPair than Session itself, check if it is really useful
  TestSessionPair p;
  EXPECT_TRUE(p.client()->isConnected());
}

TEST(TestSession, MultipleConnectionToNonReachableSd)
{
  static const auto tryTotal = 3;
  auto session = qi::makeSession();
  const auto url = "tcp://127.0.0.1:1234";
  for (int tryCount = 0; tryCount < tryTotal; ++tryCount)
  {
    ASSERT_TRUE(finishesWithError(session->connect(url)));
  }
}

TEST(TestSession, ConnectOnSecondAttempt)
{
  auto client = qi::makeSession();
  const auto url = "tcp://127.0.0.1:1234";
  ASSERT_TRUE(finishesWithError(client->connect(url)));
  ASSERT_FALSE(client->isConnected());

  auto server = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(server->listenStandalone(test::defaultListenUrl())));
  ASSERT_FALSE(server->endpoints().empty());

  ASSERT_TRUE(finishesWithValue(client->connect(test::url(*server))));
  ASSERT_TRUE(client->isConnected());
}

TEST(TestSession, MultipleConnectSuccess)
{
  auto server = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(server->listenStandalone(test::defaultListenUrl())));
  ASSERT_FALSE(server->endpoints().empty());
  auto endpoint = test::url(*server);

  auto client = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(client->connect(endpoint)));
  ASSERT_TRUE(client->isConnected());

  ASSERT_TRUE(finishesWithValue(client->close()));
  ASSERT_FALSE(client->isConnected());

  ASSERT_TRUE(finishesWithValue(client->connect(endpoint)));
  ASSERT_TRUE(client->isConnected());

  ASSERT_TRUE(finishesWithValue(client->close()));
  ASSERT_FALSE(client->isConnected());

  ASSERT_TRUE(finishesWithValue(client->connect(endpoint)));
  ASSERT_TRUE(client->isConnected());
}

TEST(TestSession, SimpleConnectionToNonReachableSd)
{
  auto session = qi::makeSession();
  const auto url = "tcp://127.0.0.1:1234";
  ASSERT_TRUE(finishesWithError(session->connect(url)));
  ASSERT_FALSE(session->isConnected());

  ASSERT_TRUE(finishesWithValue(session->close()));
  ASSERT_FALSE(session->isConnected());
}

TEST(TestSession, SimpleConnectionToInvalidAddrToSd)
{
  auto session = qi::makeSession();
  const auto url = "tcp://127.0.0.1:1234";
  ASSERT_TRUE(finishesWithError(session->connect(url)));
  ASSERT_FALSE(session->isConnected());

  ASSERT_TRUE(finishesWithValue(session->close()));
  ASSERT_FALSE(session->isConnected());
}

TEST(TestSession, SimpleConnectionToInvalidSd)
{
  auto session = qi::makeSession();
  ASSERT_TRUE(finishesWithError(session->connect(invalidUrl)));
  ASSERT_FALSE(session->isConnected());

  ASSERT_TRUE(finishesWithValue(session->close()));
  ASSERT_FALSE(session->isConnected());
}

TEST(TestSession, UnregistersServiceWhenClosed)
{
  TestSessionPair sessionPair;
  auto& sd = *sessionPair.sd();
  auto& server = *sessionPair.server();

  auto obj = dummyDynamicObject();
  unsigned int serviceIndex = 0;
  ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj),
                                willAssignValue(serviceIndex)));
  ASSERT_TRUE(finishesWithValue(server.service(dummyServiceName)));

  // Wait for the service to be available on the service directory. The reason we do this is because
  // the service being accessible from the server does not imply it has been registered on the
  // service directory yet (eg when there is a gateway between the two).
  ASSERT_TRUE(finishesWithValue(sd.waitForService(dummyServiceName)));

  {
    // Wait for the service to be unregistered from the service directory.
    SignalSpy unregisteredSpy(sd.serviceUnregistered);
    auto futSignal = unregisteredSpy.waitUntil(1, defaultWaitServiceDuration).async();
    ASSERT_TRUE(finishesWithValue(server.close()));
    ASSERT_FALSE(server.isConnected());
    ASSERT_TRUE(finishesWithValue(futSignal));
    ASSERT_TRUE(futSignal.value());
    ASSERT_EQ(dummyServiceName, unregisteredSpy.lastRecord().args.at(1).toString());
  }

  ASSERT_TRUE(finishesWithError(server.services()));

  ASSERT_TRUE(finishesWithValue(server.connect(test::url(sd))));
  ASSERT_TRUE(server.isConnected());

  ASSERT_TRUE(finishesWithError(server.service(dummyServiceName)));
  ASSERT_TRUE(finishesWithError(server.unregisterService(serviceIndex)));
}

TEST(TestSession, GetSimpleService)
{
  TestSessionPair sessionPair;
  auto& server = *sessionPair.server();

  auto obj = dummyDynamicObject();
  ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj)));
  ASSERT_TRUE(finishesWithValue(server.service(dummyServiceName)));
}

TEST(TestSession, GetSimpleServiceTwice)
{
  TestSessionPair sessionPair;
  auto& server = *sessionPair.server();
  auto& client = *sessionPair.client();

  auto obj = dummyDynamicObject();
  ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj)));

  AnyObject obj1;
  AnyObject obj2;
  ASSERT_TRUE(finishesWithValue(client.service(dummyServiceName), willAssignValue(obj1)));
  ASSERT_TRUE(finishesWithValue(client.service(dummyServiceName), willAssignValue(obj2)));

  ASSERT_TRUE(obj1.asGenericObject() == obj2.asGenericObject());
}

TEST(TestSession, GetSimpleServiceTwiceUnexisting)
{
  TestSessionPair sessionPair;
  auto& client = *sessionPair.client();
  ASSERT_TRUE(finishesWithError(client.service(dummyServiceName)));
  ASSERT_TRUE(finishesWithError(client.service(dummyServiceName)));
}

TEST(TestSession, GetUnregisterService)
{
  TestSessionPair sessionPair;
  auto& client = *sessionPair.client();
  ASSERT_TRUE(finishesWithError(client.service(dummyServiceName)));
}

TEST(TestSession, GetCloseService)
{
  TestSessionPair sessionPair;
  auto& server = *sessionPair.server();
  auto& client = *sessionPair.client();

  auto obj = dummyDynamicObject();
  ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj)));
  ASSERT_TRUE(finishesWithValue(server.close()));
  ASSERT_FALSE(server.isConnected());
  ASSERT_TRUE(finishesWithError(client.service(dummyServiceName)));
}

TEST(TestSession, AlreadyRegistered)
{
  TestSessionPair sessionPair;
  auto& server = *sessionPair.server();

  auto obj = dummyDynamicObject();
  unsigned int serviceIndex = 0;
  ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj),
                                willAssignValue(serviceIndex)));
  ASSERT_GE(serviceIndex, 0u);
  ASSERT_TRUE(finishesWithError(server.registerService(dummyServiceName, obj)));
}

TEST(TestSession, Services)
{
  TestSessionPair sessionPair;
  auto& server = *sessionPair.server();
  auto& client = *sessionPair.client();

  auto obj = dummyDynamicObject();
  ASSERT_TRUE(finishesWithValue(server.registerService("srv1.1", obj)));
  ASSERT_TRUE(finishesWithValue(server.registerService("srv1.2", obj)));
  ASSERT_TRUE(finishesWithValue(server.registerService("srv2.1", obj)));
  ASSERT_TRUE(finishesWithValue(server.registerService("srv2.2", obj)));

  auto serverServices = server.services();
  auto clientServices = client.services();

  // ServiceDirectory is listed too
  ASSERT_EQ(5u, serverServices.value().size());
  ASSERT_EQ(5u, clientServices.value().size());
}

TEST(TestSession, ServiceDirectoryEndpointsAreValid)
{
  auto session = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(session->listenStandalone("tcp://0.0.0.0:0")));
  ASSERT_FALSE(session->endpoints().empty());

  // The first endpoint has to be an accessible address/port.
  ASSERT_NE(session->endpoints().at(0).host(), "0.0.0.0");
  ASSERT_NE(session->endpoints().at(0).port(), 0);
}

TEST(TestSession, GetCallInConnect)
{
  TestSessionPair sessionPair;
  auto& sd = *sessionPair.sd();
  auto& server = *sessionPair.server();
  auto client = sessionPair.client();

  auto obj = dummyDynamicObject();
  ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj)));

  AnyObject object;
  ASSERT_TRUE(finishesWithValue(server.service(dummyServiceName), willAssignValue(object)));
  ASSERT_TRUE(object);

  client->close();
  Promise<bool> callbackFinishedPromise;
  auto weakClient = ka::weak_ptr(client);
  client->connected.connect([=]() mutable {
    auto client = weakClient.lock();
    if (!client)
      return;

    const bool result = client->services().hasValue();
    callbackFinishedPromise.setValue(result);
  });
  ASSERT_TRUE(finishesWithValue(client->connect(test::url(sd))));

  auto futureResult = callbackFinishedPromise.future();
  ASSERT_TRUE(finishesWithValue(futureResult) && futureResult.value());
}

TEST(TestSession, SignalConnectedDisconnectedNotSend)
{
  auto session = makeSession();
  SignalSpy connectedSpy{ session->connected };
  SignalSpy disconnectedSpy{ session->disconnected };

  ASSERT_TRUE(finishesWithError(session->connect(invalidUrl)));
  ASSERT_FALSE(session->isConnected());

  ASSERT_EQ(0u, connectedSpy.recordCount());
  ASSERT_EQ(0u, disconnectedSpy.recordCount());
}

TEST(TestSession, SignalConnectedDisconnectedSend)
{
  auto server = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(server->listenStandalone(test::defaultListenUrl())));
  ASSERT_FALSE(server->endpoints().empty());

  auto client = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(client->connect(test::url(*server))));
  ASSERT_TRUE(client->isConnected());

  SignalSpy disconnectedSpy{ client->disconnected };
  ASSERT_EQ(0u, disconnectedSpy.recordCount());

  ASSERT_TRUE(finishesWithValue(client->close()));
  ASSERT_FALSE(client->isConnected());

  // The disconnected signal is asynchronous, it can arrive late
  std::this_thread::sleep_for(defaultWaitDisconnectedSignalDuration);
  ASSERT_EQ(1u, disconnectedSpy.recordCount());
}

TEST(TestSession, AsyncConnect)
{
  // TODO: this test might be redundant with GetUnregisterService
  auto server = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(server->listenStandalone(test::defaultListenUrl())));
  ASSERT_FALSE(server->endpoints().empty());

  auto client = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(client->connect(test::url(*server))));
  ASSERT_TRUE(client->isConnected());
  ASSERT_TRUE(finishesWithError(client->service("IDontWantToSegfaultHere")));
}

TEST(TestSession, UrlOnClosed)
{
  auto server = qi::makeSession();
  ASSERT_TRUE(finishesWithValue(server->listenStandalone(test::defaultListenUrl())));
  ASSERT_FALSE(server->endpoints().empty());

  auto client = qi::makeSession();
  ASSERT_ANY_THROW(client->url());

  ASSERT_TRUE(finishesWithValue(client->connect(test::url(*server))));
  ASSERT_TRUE(client->isConnected());
  ASSERT_NO_THROW(client->url());

  ASSERT_TRUE(finishesWithValue(client->close()));
  ASSERT_FALSE(client->isConnected());
  ASSERT_ANY_THROW(client->url());
}

TEST(TestSession, ServiceRegisteredCtrl)
{
  // Control test for the test serviceRegistered, to ensure we properly detect
  // remote services
  TestSessionPair sessionPair;
  auto& server = *sessionPair.server();
  auto& client = *sessionPair.client();

  auto obj = dummyDynamicObject();
  ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj)));

  {
    AnyObject c;
    ASSERT_TRUE(finishesWithValue(client.service(dummyServiceName), willAssignValue(c)));
    ASSERT_TRUE(c);
    // only if we're not testing in direct mode should the objects be different
    if (sessionPair.mode() != TestMode::Mode_Direct)
    {
      auto dobj = reinterpret_cast<DynamicObject*>(c.asGenericObject()->value);
      auto sdobj = reinterpret_cast<DynamicObject*>(obj.asGenericObject()->value);
      ASSERT_NE(dobj, sdobj);
    }
  }

  {
    AnyObject c;
    ASSERT_TRUE(finishesWithValue(server.service(dummyServiceName), willAssignValue(c)));
    ASSERT_TRUE(c);
    const auto dobj = reinterpret_cast<DynamicObject*>(c.asGenericObject()->value);
    const auto sdobj = reinterpret_cast<DynamicObject*>(obj.asGenericObject()->value);
    ASSERT_EQ(dobj, sdobj);
  }
}

TEST(TestSession, ServiceRegistered)
{
  // Check a nasty race situation where a service is advertised as registered
  // by the session before being realy present
  // The symptom is not a session.service() failure, but a spurious use of
  // remote mode.
  TestSessionPair sessionPair;
  auto& server = *sessionPair.server();

  ASSERT_TRUE(finishesWithValue(server.listenStandalone(test::defaultListenUrl())));
  ASSERT_FALSE(server.endpoints().empty());

  AnyObject ao;
  Promise<void> objectSetPromise;
  server.serviceRegistered.connect([&](unsigned int, std::string name){
    ao = server.service(name).value();
    objectSetPromise.setValue(nullptr);
  });

  auto obj = dummyDynamicObject();
  ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj)));
  ASSERT_TRUE(finishesWithValue(objectSetPromise.future()));

  // check we got the object, and that it is not a remoteobject
  ASSERT_TRUE(ao);
  const auto aoGoVal = ao.asGenericObject()->value;
  ASSERT_TRUE(aoGoVal);
  ASSERT_EQ(obj.asGenericObject()->value, aoGoVal);
}

TEST(TestSession, RegisterServiceFromClient)
{
  TestSessionPair sessionPair;
  auto& client = *sessionPair.client();
  auto obj = dummyDynamicObject();
  ASSERT_TRUE(finishesWithValue(client.listen("tcp://localhost:0").async()));
  ASSERT_TRUE(finishesWithValue(client.registerService(dummyServiceName, obj)));

  AnyObject object;
  ASSERT_TRUE(finishesWithValue(client.service(dummyServiceName), willAssignValue(object)));
  ASSERT_TRUE(object);
  ASSERT_EQ("foo", object.call<std::string>("reply", "foo"));
}

TEST(TestSession, WaitForService)
{
  TestSessionPair sessionPair;
  auto& server = *sessionPair.server();
  auto& client = *sessionPair.client();

  auto obj = dummyDynamicObject();

  unsigned int sid = 0;
  ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj), willAssignValue(sid)));
  ASSERT_TRUE(finishesWithValue(client.waitForService(dummyServiceName)));
  ASSERT_TRUE(finishesWithValue(server.unregisterService(sid)));
  auto future = client.waitForService(dummyServiceName);
  ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj), willAssignValue(sid)));
  ASSERT_TRUE(finishesWithValue(future));
}

TEST(TestSession, WaitForServiceCanceled)
{
  TestSessionPair sessionPair;
  auto& client = *sessionPair.client();

  auto future = client.waitForService(dummyServiceName);
  future.cancel();
  ASSERT_TRUE(finishesAsCanceled(future));
}

TEST(TestSession, EndpointsAreOrderedByPreference)
{
  using namespace testing;
  auto session = qi::makeSession();
  session->listen("tcp://0.0.0.0:0");
  // We expect more than one endpoint, otherwise the test is pointless and it
  // might fail silently.
  EXPECT_GT(session->endpoints().size(), 1u);
  const auto endpoints = session->endpoints();
  const auto isPreferredEndpoint = [](const qi::Url& u1, const qi::Url& u2) {
    return qi::isPreferredEndpoint(*toUri(u1), *toUri(u2));
  };
  EXPECT_THAT(endpoints, WhenSortedBy(isPreferredEndpoint, Eq(endpoints)));
}
