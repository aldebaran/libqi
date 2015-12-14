/*
 ** Author(s):
 **  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <string>

#include <boost/thread/mutex.hpp>

#include <gtest/gtest.h>

#include <qi/anyobject.hpp>
#include <qi/application.hpp>
#include <qi/session.hpp>
#include <qi/future.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/log.hpp>

qiLogCategory("TestGateway");

namespace
{

  using qi::SessionPtr;

  class TestGateway : public ::testing::Test
  {
  public:
    void SetUp()
    {
      sd_.listenStandalone("tcp://127.0.0.1:0");
      qi::Future<void> fut = gw_.attachToServiceDirectory(sd_.url());
      fut.wait();
      if (fut.hasError())
      {
        qiLogError() << "error: " << fut.error();
        ASSERT_TRUE(false);
      }
      gw_.listen("tcp://127.0.0.1:0");
    }

    void registerSdService(const std::string& serviceName);
    void registerSdService(const std::string& serviceName, qi::AnyObject service);
    qi::SessionPtr connectClientToSd();
    qi::SessionPtr connectClientToGw();

    qi::Gateway gw_;
    qi::Session sd_;
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

  void TestGateway::registerSdService(const std::string& serviceName)
  {
    registerSdService(serviceName, makeBaseService());
  }

  void TestGateway::registerSdService(const std::string& serviceName, qi::AnyObject service)
  {
    sd_.registerService(serviceName, service);
  }


  qi::SessionPtr TestGateway::connectClientToSd()
  {
    qi::SessionPtr session = qi::makeSession();
    session->connect(sd_.url());
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
      : prom_(prom), exVal_(expectedValue), remainingCalls_(expectedCalls), ov_(hasOverflowed) {}

    callsync_(const callsync_& cs)
      : prom_(cs.prom_), exVal_(cs.exVal_), remainingCalls_(cs.remainingCalls_), ov_(cs.ov_)
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

  static void reco_sync(qi::Promise<void> prom)
  {
    prom.setValue(0);
  }

  TEST_F(TestGateway, testSimpleMethodCallGwService)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();

    serviceHost->registerService("my_service", makeBaseService());
    qi::AnyObject service = client->service("my_service");
    int value = rand();

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

    qi::AnyObject service = client->service("my_service");
    int value = rand();
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
    qi::Promise<int> sync;
    qi::Future<int> fut = sync.future();

    serviceHost->serviceRegistered.connect(&serviceRegistered, _1, sync);
    serviceHost->registerService("my_service", makeBaseService());
    fut.wait();
    //sync.reset();
    sync = qi::Promise<int>();
    fut = sync.future();
    qi::AnyObject service = client->service("my_service");
    int value = rand();
    service.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value)));
    service.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());

    int res = service.call<int>("echoValue", value);
    ASSERT_EQ(res, value);
  }

  TEST_F(TestGateway, testNoSuchService)
  {
    SessionPtr client = connectClientToGw();
    SessionPtr serviceHost = connectClientToSd();
    bool success = false;
    qi::AnyObject service;

    try {
      service = client->service("my_service");
    } catch (const std::runtime_error&) {
      success = true;
    }
    ASSERT_TRUE(success);
    success = false;

    int id = serviceHost->registerService("my_service", makeBaseService());
    service = client->service("my_service");
    ASSERT_EQ(service.call<int>("echoValue", 44), 44);
    serviceHost->unregisterService(id);
    try {
      service.call<int>("echoValue", 44);
    } catch (const std::exception&) {
      success = true;
    }
    ASSERT_TRUE(success);
  }

  TEST_F(TestGateway, testSignalsProperlyDisconnected)
  {
    SessionPtr client = connectClientToGw();
    SessionPtr serviceHost = connectClientToGw();
    int value = rand();
    callsync_ callsync(qi::Promise<int>(), value, 1);
    qi::Promise<int> prom;
    qi::Future<int> fut = callsync.prom_.future();

    serviceHost->registerService("my_service", makeBaseService());
    qi::AnyObject service = client->service("my_service");

    qi::SignalLink link = service.connect("echoSignal", boost::function<void(int)>(callsyncwrap_(&callsync)));
    service.connect("echoSignal", boost::function<void(int)>(boost::bind(&qi::Promise<int>::setValue, &prom, _1)));
    service.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
    ASSERT_EQ(callsync.remainingCalls_, 0);

    // Disconnect the signal and check we don't receive it anymore
    // fut ensures we still receive the signal properly
    //callsync.prom_.reset();
    //prom.reset();
    callsync.prom_ = qi::Promise<int>();
    prom = qi::Promise<int>();
    fut = prom.future();
    callsync.remainingCalls_ = 1;
    service.disconnect(link);
    service.post("echoSignal", value);
    fut.wait();
    ASSERT_EQ(callsync.remainingCalls_, 1);

    // Reconnect the signal, disconnect the client, reconnect the client,
    // trigger the signal : we should receive it only once (links properly
    // disconnected GW-side).
    callsync.remainingCalls_ = 2;
    fut = callsync.prom_.future();
    service.connect("echoSignal", boost::function<void(int)>(callsyncwrap_(&callsync)));
    client->close();
    client = connectClientToGw();
    service = client->service("my_service");
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
    int value = rand();

    serviceHost->registerService("my_service", makeBaseService());
    for (int i = 0; i < 5; ++i)
      clients[i] = connectClientToGw();
    for (int i = 0; i < 5; ++i)
      serviceObjects[i] = clients[i]->service("my_service");
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
    int value = rand();
    bool overflow = false;
    callsync_ callsync(prom, value, 5, &overflow);

    serviceHost->registerService("my_service", makeBaseService());
    for (int i = 0; i < 5; ++i)
      clients[i] = connectClientToGw();
    for (int i = 0; i < 5; ++i)
      serviceObjects[i] = clients[i]->service("my_service");
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

  static void disco_sync(qi::Promise<void> prom, int* atomix, boost::mutex* moutex)
  {
    boost::mutex::scoped_lock lock(*moutex);
    ++*atomix;
    if (*atomix == 2)
    {
      prom.setValue(0);
    }
  }

  TEST_F(TestGateway, testOnSDDeathGwReconnectsAndStillWorksProperly)
  {
    SessionPtr serviceHost = connectClientToGw();
    SessionPtr client = connectClientToGw();
    qi::Session nextSD;
    int count = 0;
    boost::mutex moutex;
    qi::Promise<void> sync;
    qi::AnyObject service;
    qi::Url origUrl = sd_.url();

    qi::SignalLink shl = serviceHost->disconnected.connect(&disco_sync, sync, &count, &moutex);
    qi::SignalLink cl = client->disconnected.connect(&disco_sync, sync, &count, &moutex);
    serviceHost->registerService("my_service", makeBaseService());
    service = client->service("my_service");
    int value = rand();

    ASSERT_EQ(service.call<int>("echoValue", value), value);
    sd_.close();
    sync.future().wait();

    //sync.reset();
    sync = qi::Promise<void>();
    gw_.connected.connect(reco_sync, sync);
    nextSD.listenStandalone(origUrl);
    sync.future().wait();

    serviceHost->connect(gw_.endpoints()[0]);
    client->connect(gw_.endpoints()[0]);
    serviceHost->registerService("my_service", makeBaseService());
    service = client->service("my_service");
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
    qi::AnyObject service = client->service("my_service");
    int value = rand();
    qi::SignalLink link = service.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value)));
    service.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
    //sync.reset();
    sync = qi::Promise<int>();
    qi::Future<void> fut2 = service.disconnect(link);
    ASSERT_FALSE(fut2.hasError());

    link = service.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value)));
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
    qi::AnyObject service = client->service("my_service");
    qi::AnyObject danglingObject = service.call<qi::AnyObject>("getObject");


    // Test Call
    int value = rand();
    int tentative = danglingObject.call<int>("echoValue", value);
    ASSERT_EQ(tentative, value);

    // TestSignals
    value = rand();
    qi::SignalLink link = danglingObject.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value)));
    danglingObject.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
    sync = qi::Promise<int>();
    fut = sync.future();
    qi::Future<void> fut2 = danglingObject.disconnect(link);
    ASSERT_FALSE(fut2.hasError());
    //qi::os::sleep(2);
    link = danglingObject.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value)));
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
    qi::AnyObject service = client->service("my_service");
    qi::AnyObject clientHostedObject = makeBaseService();
    service.call<void>("supplyObject", clientHostedObject);
    qi::AnyObject danglingObject = concreteService->getSuppliedObject();

    // Test Call
    int value = rand();
    int tentative = danglingObject.call<int>("echoValue", value);
    ASSERT_EQ(tentative, value);


    // TestSignals
    qi::Promise<int> sync;
    qi::Future<int> fut = sync.future();
    value = rand();
    qi::SignalLink link = danglingObject.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value)));
    danglingObject.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
    //sync.reset();
    sync = qi::Promise<int>();
    fut = sync.future();
    qi::Future<void> fut2 = danglingObject.disconnect(link);
    ASSERT_FALSE(fut2.hasError());
    link = danglingObject.connect("echoSignal", boost::function<void (int)>(callsync_(sync, value)));
    danglingObject.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
  }

}

int main(int ac, char **av)
{
  qi::Application app(ac, av);
  ::testing::InitGoogleTest(&ac, av);
  srand(time(NULL));
  return RUN_ALL_TESTS();
}
