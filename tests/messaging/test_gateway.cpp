/*
 ** Author(s):
 **  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <string>
#include <random>

#include <boost/thread/mutex.hpp>

#include <gtest/gtest.h>

#include <qi/anyobject.hpp>
#include <qi/application.hpp>
#include <qi/session.hpp>
#include <qi/future.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/log.hpp>
#include <qi/testutils/testutils.hpp>

qiLogCategory("TestGateway");

namespace qi
{

  template<typename T>
  std::ostream& operator<<(std::ostream& os, const qi::Object<T>& obj)
  {
    if (obj.isValid())
      os << obj.uid() << "\n";
    else
      os << "<INVALID OBJECT>\n";
    return os;
  }

}

namespace
{

  using qi::SessionPtr;

  class TestGateway : public ::testing::Test
  {
  public:
    TestGateway()
      : randEngine{ [] {
        std::random_device rd;
        std::seed_seq seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
        return std::default_random_engine{ seq };
      }() }
      , sd_{ qi::makeSession() }
    {}

    void SetUp()
    {
      sd_->listenStandalone("tcp://127.0.0.1:0");
      gw_.attachToServiceDirectory(sd_->url()).value(); // throw on error
      const auto listenStatus = gw_.listenAsync("tcp://127.0.0.1:0").value();
      ASSERT_EQ(listenStatus, qi::Gateway::ListenStatus::Listening);
    }

    int randomValue()
    {
      return intDistrib(randEngine);
    }

    qi::SessionPtr connectClientToSd();
    qi::SessionPtr connectClientToGw();

    std::default_random_engine randEngine;
    std::uniform_int_distribution<int> intDistrib;
    qi::Gateway gw_;
    qi::SessionPtr sd_;
  };

  int echoValue(int value)
  {
    qiLogInfo() << "echovalue: " << value;
    return value;
  }

  static qi::AnyObject makeBaseService();
  qi::AnyObject getObject()
  {
    return makeBaseService();
  }

  static qi::AnyObject makeBaseService()
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseMethod<int (int)>("echoValue", &echoValue);
    ob.advertiseMethod<qi::AnyObject (void)>("getObject", &getObject);
    ob.advertiseSignal<int>("echoSignal");
    ob.advertiseSignal<int>("echoSignal2");
    return ob.object();
  }

  qi::SessionPtr TestGateway::connectClientToSd()
  {
    qi::SessionPtr session = qi::makeSession();
    session->connect(sd_->url());
    return session;
  }

  qi::SessionPtr TestGateway::connectClientToGw()
  {
    qi::SessionPtr session = qi::makeSession();
    session->connect(gw_.endpoints().at(0));
    return session;
  }

  struct callsync_
  {
    callsync_(qi::Promise<int> prom, int expectedValue, int expectedCalls = 1, bool* hasOverflowed = NULL)
      : prom_(prom), exVal_(expectedValue), remainingCalls_(expectedCalls), ov_(hasOverflowed)
    {}

    callsync_(const callsync_& cs)
      : prom_(cs.prom_), exVal_(cs.exVal_), remainingCalls_(cs.remainingCalls_), ov_(cs.ov_), moutecks_()
    {}

    void operator()(int value)
    {
      boost::mutex::scoped_lock lock(moutecks_);
      qiLogInfo() << "Called:" << value;
      if (value != exVal_)
      {
        std::stringstream builder;
        builder << "Expected " << exVal_ << ", received " << value << std::endl;
        prom_.setError(builder.str());
      }
      else if (!--remainingCalls_)
        prom_.setValue(value);
      else if (remainingCalls_ < 0 && ov_)
        *ov_ = true;
    }
    qi::Promise<int> prom_;
    int exVal_;
    int remainingCalls_;
    bool* ov_;
    boost::mutex moutecks_;
  };

  struct callsyncwrap_
  {
    callsyncwrap_(callsync_* wrap) : wrapped_(wrap) {}
    callsyncwrap_(const callsyncwrap_& w) : wrapped_(w.wrapped_) {}
    void operator() (int value)
    {
      (*wrapped_)(value);
    }
    callsync_* wrapped_;
  };

  TEST_F(TestGateway, testSimpleMethodCallGwService)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();

    serviceHost->registerService("my_service", makeBaseService());
    qi::AnyObject service = client->service("my_service").value();
    int value = randomValue();

    ASSERT_EQ(service.call<int>("echoValue", value), value);
    client->close();
    serviceHost->close();
  }

  TEST_F(TestGateway, testSimpleSignalGwService)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();
    qi::Promise<int> sync;
    qi::Future<int> fut = sync.future();

    serviceHost->registerService("my_service", makeBaseService());

    qi::AnyObject service = client->service("my_service").value();
    int value = randomValue();
    service.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value)));
    service.post("echoSignal", value);
    fut.wait();

    ASSERT_FALSE(fut.hasError());
  }

  static void serviceRegistered(unsigned int id, qi::Promise<int> prom)
  {
    prom.setValue(id);
  }

  TEST_F(TestGateway, testSDLocalService)
  {
    SessionPtr client = connectClientToGw();
    SessionPtr serviceHost = connectClientToSd();

    {
      qi::Promise<int> sync;
      serviceHost->serviceRegistered.connect(&serviceRegistered, _1, sync);
      serviceHost->registerService("my_service", makeBaseService());
      sync.future().wait();
    }

    qi::AnyObject service;
    const int value = randomValue();
    {
      qi::Promise<int> sync;
      auto fut = sync.future();
      ASSERT_TRUE(test::finishesWithValue(client->waitForService( "my_service" )));
      service = client->service("my_service").value();
      service.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value)));
      service.post("echoSignal", value);
      fut.wait();
      ASSERT_FALSE(fut.hasError());
    }

    int res = service.call<int>("echoValue", value);
    ASSERT_EQ(res, value);
  }

  TEST_F(TestGateway, testNoSuchService)
  {
    SessionPtr client = connectClientToGw();
    SessionPtr serviceHost = connectClientToSd();

    qi::AnyObject service;
    ASSERT_ANY_THROW(service = client->service("my_service").value());

    const auto id = serviceHost->registerService("my_service", makeBaseService()).value();
    ASSERT_TRUE(test::finishesWithValue(client->waitForService("my_service")));
    service = client->service("my_service").value();
    ASSERT_EQ(service.call<int>("echoValue", 44), 44);
    serviceHost->unregisterService(id);
    ASSERT_ANY_THROW(service.call<int>("echoValue", 44));
  }

  TEST_F(TestGateway, testSignalsProperlyDisconnected)
  {
    SessionPtr client = connectClientToGw();
    SessionPtr serviceHost = connectClientToGw();
    int value = randomValue();

    serviceHost->registerService("my_service", makeBaseService());
    qi::AnyObject service = client->service("my_service").value();

    callsync_ callsync(qi::Promise<int>(), value, 1);
    qi::Future<int> fut = callsync.prom_.future();
    qi::SignalLink callsyncOnEchoLink = service.connect("echoSignal", [&](int value){ callsync(value); }).value();
    service.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
    ASSERT_EQ(callsync.remainingCalls_, 0);

    // Disconnect the signal and check we don't receive it anymore
    // fut ensures we still receive the signal properly
    service.disconnect(callsyncOnEchoLink);
    callsync.prom_ = qi::Promise<int>();
    callsync.remainingCalls_ = 1;

    qi::Promise<int> witnessPromise;
    fut = witnessPromise.future();
    qi::SignalLink setValueOnEchoLink =
        service.connect("echoSignal", [&](int v){ witnessPromise.setValue(v); }).value();

    service.post("echoSignal", value);
    fut.wait();
    service.disconnect(setValueOnEchoLink);
    ASSERT_EQ(callsync.remainingCalls_, 1);

    // Reconnect the signal, disconnect the client, reconnect the client,
    // trigger the signal : we should receive it only once (links properly
    // disconnected GW-side).
    callsync.remainingCalls_ = 2;
    fut = callsync.prom_.future();
    service.connect("echoSignal", boost::function<void(int)>(callsyncwrap_(&callsync)));
    client->close();
    client = connectClientToGw();
    service = client->service("my_service").value();
    service.connect("echoSignal", boost::function<void(int)>(callsyncwrap_(&callsync)));
    service.post("echoSignal", value);
    service.post("echoSignal", value);
    fut.wait();
    ASSERT_EQ(callsync.remainingCalls_, 0);
    ASSERT_FALSE(fut.hasError());
  }

  TEST_F(TestGateway, testFunctionMultiUser)
  {
    SessionPtr serviceHost = connectClientToGw();
    SessionPtr clients[5] = {};
    qi::AnyObject serviceObjects[5] = {};
    int value = randomValue();

    serviceHost->registerService("my_service", makeBaseService());
    for (int i = 0; i < 5; ++i)
      clients[i] = connectClientToGw();
    for (int i = 0; i < 5; ++i)
      serviceObjects[i] = clients[i]->service("my_service").value();
    for (int i = 0; i < 5; ++i)
      ASSERT_EQ(serviceObjects[i].call<int>("echoValue", value), value);
    for (int i = 0; i < 5; ++i)
      clients[i]->close();
    serviceHost->close();
  }

  TEST_F(TestGateway, testSignalsMultiUser)
  {
    SessionPtr serviceHost = connectClientToGw();
    SessionPtr clients[5] = {};
    qi::AnyObject serviceObjects[5] = {};
    qi::Promise<int> prom;
    qi::Future<int> fut = prom.future();
    int value = randomValue();
    bool overflow = false;
    callsync_ callsync(prom, value, 5, &overflow);

    serviceHost->registerService("my_service", makeBaseService());
    for (int i = 0; i < 5; ++i)
      clients[i] = connectClientToGw();
    for (int i = 0; i < 5; ++i)
      serviceObjects[i] = clients[i]->service("my_service").value();
    for (int i = 0; i < 5; ++i)
      serviceObjects[i].connect("echoSignal", boost::function<void(int)>(callsyncwrap_(&callsync)));
    serviceObjects[0].post("echoSignal", value);

    fut.wait();
    ASSERT_FALSE(fut.hasError());
    ASSERT_FALSE(overflow);
    ASSERT_EQ(callsync.remainingCalls_, 0);
    for (int i = 0; i <5 ; ++i)
      clients[i]->close();
  }

  void setPromiseIfCountEquals(qi::Promise<void> prom, std::atomic<int>& count, int value)
  {
    if (++count == value)
    {
      prom.setValue(nullptr);
    }
  }

  TEST_F(TestGateway, testOnSDDeathGwReconnectsAndStillWorksProperly)
  {
    SessionPtr serviceHost = connectClientToGw();
    SessionPtr client = connectClientToGw();
    auto nextSD = qi::makeSession();
    std::atomic<int> count{ 0 };
    qi::AnyObject service;
    qi::Url origUrl = sd_->url();

    qi::Promise<void> sync;
    qi::SignalLink shl = serviceHost->disconnected.connect(setPromiseIfCountEquals, sync, std::ref(count), 2);
    qi::SignalLink cl = client->disconnected.connect(setPromiseIfCountEquals, sync, std::ref(count), 2);
    serviceHost->registerService("my_service", makeBaseService());
    service = client->service("my_service").value();
    int value = randomValue();

    ASSERT_EQ(service.call<int>("echoValue", value), value);
    sd_->close();
    sync.future().wait();

    {
      qi::Promise<void> sync;
      gw_.status.connect([&](const qi::Gateway::Status& status){
        if(status.isReady())
          sync.setValue(nullptr);
      });
      nextSD->listenStandalone(origUrl);
      sync.future().wait();
    }

    const auto gatewayEndpoints = gw_.endpoints();
    ASSERT_FALSE(gatewayEndpoints.empty());
    const auto& firstEndpoint = gatewayEndpoints[0];
    serviceHost->connect(firstEndpoint);
    client->connect(firstEndpoint);
    serviceHost->registerService("my_service", makeBaseService());
    service = client->service("my_service").value();
    ASSERT_EQ(service.call<int>("echoValue", value), value);
    serviceHost->disconnected.disconnect(shl);
    client->disconnected.disconnect(cl);
    client->close();
    serviceHost->close();
  }

  TEST_F(TestGateway, testUnregisterSignal)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();
    qi::Promise<int> sync;
    qi::Future<int> fut = sync.future();

    serviceHost->registerService("my_service", makeBaseService());
    qi::AnyObject service = client->service("my_service").value();
    int value = randomValue();
    qi::SignalLink link = service.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value))).value();
    service.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
    //sync.reset();
    sync = qi::Promise<int>();
    qi::Future<void> fut2 = service.disconnect(link);
    ASSERT_FALSE(fut2.hasError());

    link = service.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value))).value();
    service.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
  }

  TEST_F(TestGateway, testDanglingObjectsClientService)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();
    qi::Promise<int> sync;
    qi::Future<int> fut = sync.future();

    serviceHost->registerService("my_service", makeBaseService());
    qi::AnyObject service = client->service("my_service").value();
    qi::AnyObject danglingObject = service.call<qi::AnyObject>("getObject");


    // Test Call
    int value = randomValue();
    int tentative = danglingObject.call<int>("echoValue", value);
    ASSERT_EQ(tentative, value);

    // TestSignals
    value = randomValue();
    qi::SignalLink link = danglingObject.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value))).value();
    danglingObject.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
    sync = qi::Promise<int>();
    fut = sync.future();
    qi::Future<void> fut2 = danglingObject.disconnect(link);
    ASSERT_FALSE(fut2.hasError());
    //qi::os::sleep(2);
    link = danglingObject.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value))).value();
    danglingObject.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());

  }

  // In this test, the client is handing an object to the service.

  class ObjectUserService
  {
  public:
    void supplyObject(qi::AnyObject obj);
    qi::AnyObject getSuppliedObject();

  private:
    qi::AnyObject clientSuppliedObject;
  };
  void ObjectUserService::supplyObject(qi::AnyObject obj)
  {
    clientSuppliedObject = obj;
  }
  qi::AnyObject ObjectUserService::getSuppliedObject()
  {
    return clientSuppliedObject;
  }
  QI_REGISTER_OBJECT(ObjectUserService, supplyObject);

  TEST_F(TestGateway, testDanglingObjectsServiceClient)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();
    qi::Object<ObjectUserService> concreteService(new ObjectUserService);

    serviceHost->registerService("my_service", concreteService);
    qi::AnyObject service = client->service("my_service").value();
    qi::AnyObject clientHostedObject = makeBaseService();
    service.call<void>("supplyObject", clientHostedObject);
    qi::AnyObject danglingObject = concreteService->getSuppliedObject();

    // Test Call
    int value = randomValue();
    int tentative = danglingObject.call<int>("echoValue", value);
    ASSERT_EQ(tentative, value);


    // TestSignals
    qi::Promise<int> sync;
    qi::Future<int> fut = sync.future();
    value = randomValue();
    qi::SignalLink link = danglingObject.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value))).value();
    danglingObject.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
    //sync.reset();
    sync = qi::Promise<int>();
    fut = sync.future();
    qi::Future<void> fut2 = danglingObject.disconnect(link);
    ASSERT_FALSE(fut2.hasError());
    link = danglingObject.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value))).value();
    danglingObject.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
  }

  TEST_F(TestGateway, RegisterServiceOnGWRegistersItOnSD)
  {
    auto gwServer = connectClientToGw();
    auto sdClient = connectClientToSd();

    const auto serviceName = "my_service";
    qi::Object<ObjectUserService> concreteService(boost::make_shared<ObjectUserService>());
    gwServer->registerService(serviceName, concreteService);

    qi::AnyObject serviceObject;
    ASSERT_TRUE(test::finishesWithValue(sdClient->waitForService(serviceName)));
    ASSERT_TRUE(test::finishesWithValue(sdClient->service(serviceName),
                                        test::willAssignValue(serviceObject)));
    ASSERT_TRUE(serviceObject.isValid());

    // TODO: It would be good if this worked but right now these two objects don't have the same
    // ObjectUid.
    // ASSERT_EQ(concreteService, serviceObject);
  }

  TEST_F(TestGateway, ServiceRegisteredOnGWIsAvailableOnGW)
  {
    auto gwServer = connectClientToGw();
    auto gwClient = connectClientToGw();

    const auto serviceName = "my_service";
    qi::Object<ObjectUserService> concreteService(boost::make_shared<ObjectUserService>());
    gwServer->registerService(serviceName, concreteService);

    qi::AnyObject serviceObject;
    ASSERT_TRUE(test::finishesWithValue(gwClient->waitForService(serviceName)));
    ASSERT_TRUE(test::finishesWithValue(gwClient->service(serviceName),
                                        test::willAssignValue(serviceObject)));
    ASSERT_TRUE(serviceObject.isValid());

    // TODO: It would be good if this worked but right now these two objects don't have the same
    // ObjectUid.
    // ASSERT_EQ(concreteService, serviceObject);
  }

  TEST(TestGatewayLateSD, AttachesToSDWhenAvailable)
  {
    qi::Gateway gw;
    auto futAttach = gw.attachToServiceDirectory("tcp://127.0.0.1:59345");
    ASSERT_TRUE(test::isStillRunning(futAttach, test::willDoNothing(), qi::Seconds{ 2 }));

    auto sd = qi::makeSession();
    sd->listenStandalone("tcp://127.0.0.1:59345");

    // It can take a while for the gateway to reconnect if the service directory has not be found
    // after a long time, so wait for a while.
    ASSERT_TRUE(test::finishesWithValue(futAttach, test::willDoNothing(), qi::Minutes{ 5 }));
    ASSERT_EQ(qi::Gateway::ConnectionStatus::Connected, gw.status.get().value().connection);
  }
}
