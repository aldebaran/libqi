/*
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <future>
#include <vector>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include <qi/session.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/dynamicobject.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <testsession/testsessionpair.hpp>
#include <boost/optional/optional_io.hpp>

qiLogCategory("test");

static const qi::MilliSeconds usualTimeout{200};

static std::string reply(const std::string &msg)
{
  return msg;
}

/* For asynchronous things where no synchronisation mechanism
 * is possible, loop the check and wait a small delay,
 * instead of one big sleep that will slow us down
 *
 */
#define PERSIST_CHECK(code, cond, what, msdelay)                               \
do                                                                             \
{                                                                              \
  code;                                                                        \
  for(unsigned i=0; i<50 && !(cond); ++i)                                      \
  {                                                                            \
    std::this_thread::sleep_for(std::chrono::milliseconds{1} + msdelay / 50 ); \
    code;                                                                      \
  }                                                                            \
  what(cond);                                                                  \
} while(0)

#define PERSIST_ASSERT(code, cond, msdelay)  \
 PERSIST_CHECK(code, cond, ASSERT_TRUE, msdelay)
#define PERSIST_EXPECT(code, cond, msdelay)  \
 PERSIST_CHECK(code, cond, EXPECT_TRUE, msdelay)
#define PERSIST(code, cond, msdelay)  \
  PERSIST_CHECK(code, cond, (void),msdelay)
//check for server closed
//check for socket disconnected
//check for service unregistered
//check for service unregistered, then readded

TEST(QiService, RemoteObjectCacheServerClose)
{
  TestSessionPair p;
  if (p.server() == p.client()) // we close and not unregister, so does not work in direct mode
    return;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  p.server()->registerService("serviceTest", obj);
  qi::Future<qi::AnyObject> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value().call<std::string>("reply", "titi"));

  p.server()->close();

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), std::chrono::milliseconds{1000});
}


TEST(QiService, RemoteObjectCacheUnregister)
{
  TestSessionPair p;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  unsigned int idx = p.server()->registerService("serviceTest", obj).value();
  qi::Future<qi::AnyObject> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value().call<std::string>("reply", "titi"));

  p.server()->unregisterService(idx);

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), std::chrono::milliseconds{1000});

}


TEST(QiService, RemoteObjectCacheABAUnregister)
{
  TestSessionPair p;

  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  unsigned int idx = p.server()->registerService("serviceTest", obj).value();
  qi::Future<qi::AnyObject> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value().call<std::string>("reply", "titi"));

  p.server()->unregisterService(idx);

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), std::chrono::milliseconds{1000});

  unsigned int idx2 = p.server()->registerService("serviceTest", obj).value();
  //new service should not have a previoulsy registered ID
  EXPECT_NE(idx2, idx);

  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  qi::Future<std::string> fret = fut.value().async<std::string>("reply", "titi");
  if (fret.hasError()) {
    std::cout << "Error returned:" << fret.error();
  }
  EXPECT_FALSE(fret.hasError());
  EXPECT_EQ(std::string("titi"), fret.value());
}


TEST(QiService, RemoteObjectCacheABANewServer)
{
  TestSessionPair p;

  auto ses = qi::makeSession();
  if (p.server() == p.client()) // we close and not unregister, so does not work in direct mode
    return;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::AnyObject obj(ob.object());

  unsigned int idx = p.server()->registerService("serviceTest", obj).value();
  qi::Future<qi::AnyObject> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value().call<std::string>("reply", "titi"));

  p.server()->close();

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), std::chrono::milliseconds{1000});

  qi::Future<void> f = ses->connect(p.client()->url().str());
  f.wait(8000);
  EXPECT_FALSE(f.hasError());
  ses->listen("tcp://0.0.0.0:0");
  unsigned int idx2 = ses->registerService("serviceTest", obj).value();
  //new service should not have a previoulsy registered ID
  EXPECT_NE(idx2, idx);

  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  qi::Future<std::string> fret = fut.value().async<std::string>("reply", "titi");
  if (fret.hasError()) {
    std::cout << "Error returned:" << fret.error();
  }
  EXPECT_FALSE(fret.hasError());
  EXPECT_EQ(std::string("titi"), fret.value());
}


TEST(QiService, RemoteObjectNackTransactionWhenServerClosed)
{
  TestSessionPair p;
  if (p.server() == p.client()) // we close and not unregister, so does not work in direct mode
    return;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("sleep", [](qi::MilliSeconds delay){ boost::this_thread::sleep_for(delay); });
  qi::AnyObject obj(ob.object());

  p.server()->registerService("serviceTest", obj);
  qi::Future<qi::AnyObject> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  qi::Future<void> fret = fut.value().async<void>("sleep", qi::Seconds{2});
  qi::Future<void> fclose = p.server()->close();
  fclose.wait(1000);
  EXPECT_TRUE(fclose.isFinished());
  EXPECT_FALSE(fclose.hasError(1));

  fret.wait(1000);
  //once the server is close, the answer should be ready.
  EXPECT_TRUE(fret.isFinished());
  //the service is closed, so it can't send an answer.
  EXPECT_TRUE(fret.hasError(1000));
}


class Foo
{
public:
  int ping(int i) { return i + prop.get().value();}
  qi::Property<int> prop;
};

// Second parameter is unused, increment step is always 1.
void inc (qi::Atomic<int>* daInt, int)
{
  ++(*daInt);
}

TEST(QiService, ClassProperty)
{
  Foo f; // foo is registered as service, so must survive the session
  TestSessionPair p;

  qi::ObjectTypeBuilder<Foo> builder;
  builder.advertiseMethod("ping", &Foo::ping);
  ASSERT_TRUE(builder.advertiseProperty("offset", &Foo::prop) > 0);

  qi::AnyObject obj = builder.object(&f, &qi::AnyObject::deleteGenericObjectOnly);

  p.server()->registerService("foo", obj);

  qi::AnyObject client = p.client()->service("foo").value();
  qi::detail::printMetaObject(std::cerr, obj.metaObject());
  std::cerr <<"--" << std::endl;
  qi::detail::printMetaObject(std::cerr, client.metaObject());
  qiLogDebug() << "setProp";
  client.setProperty<int>("offset", 1).value();
  qiLogDebug() << "setProp done";
  ASSERT_EQ(1, f.prop.get().value());
  ASSERT_EQ(2, client.call<int>("ping", 1));
  f.prop.set(2);
  ASSERT_EQ(3, client.call<int>("ping", 1));
  ASSERT_EQ(2, client.property<int>("offset").value());

  // test event
  qi::Atomic<int> hit{0};
  boost::function<void(int)> incHit = boost::bind(&inc, &hit,_1);
  f.prop.connect(incHit);
  obj.connect("offset", incHit);
  client.connect("offset", incHit);
  f.prop.set(1);
  PERSIST_ASSERT(, (hit.load()) == 3, std::chrono::milliseconds{5});
  client.setProperty("offset", 2);
  PERSIST_ASSERT(, (hit.load()) == 6, std::chrono::milliseconds{5});

  // test error handling
   EXPECT_TRUE(client.setProperty("canard", 5).hasError());
   EXPECT_TRUE(client.setProperty("offset", "astring").hasError());
}

int prop_ping(qi::PropertyBase* &p, int v)
{
  return static_cast<int>(p->value().value().toInt() + v);
}

TEST(QiService, GenericProperty)
{
  TestSessionPair p;
  qi::DynamicObject* dobj = new qi::DynamicObject();
  qi::DynamicObjectBuilder builder(dobj);
  unsigned int propId = builder.advertiseProperty<int>("offset");
  qi::PropertyBase* prop;
  builder.advertiseMethod("ping",
    (boost::function<int (int)>)boost::bind(&prop_ping, boost::ref(prop), _1));
  qi::AnyObject obj = builder.object();
  prop = dobj->property(propId);
  prop->setValue(0);
  p.server()->registerService("foo", obj);

  qi::AnyObject client = p.client()->service("foo").value();

  client.setProperty("offset", 1);
  ASSERT_EQ(1, prop->value().value().toInt());
  ASSERT_EQ(2, client.call<int>("ping", 1));
  prop->setValue(2);
  ASSERT_EQ(3, client.call<int>("ping", 1));
  ASSERT_EQ(2, client.property<int>("offset").value());

  // test event
  qi::Atomic<int> hit;
  boost::function<void(int)> incHit = boost::bind(&inc, &hit, _1);
  qiLogVerbose() << "Connecting to signal";
  ASSERT_NE(qi::SignalBase::invalidSignalLink, prop->signal()->connect(incHit));
  ASSERT_NE(qi::SignalBase::invalidSignalLink, obj.connect("offset", incHit).value());
  ASSERT_NE(qi::SignalBase::invalidSignalLink, client.connect("offset", incHit).value());
  qiLogVerbose() << "Triggering prop set";
  prop->setValue(1);
  PERSIST(, (hit.load()) == 3, std::chrono::milliseconds{500});
  std::this_thread::sleep_for(std::chrono::milliseconds{500}); \
  EXPECT_EQ(3, hit.load());
  client.setProperty<int>("offset", 2);
  PERSIST(, (hit.load()) == 6, std::chrono::milliseconds{500});
  std::this_thread::sleep_for(std::chrono::milliseconds{500}); \
  EXPECT_EQ(6, hit.load());
  if (client != obj)
  {
    client.call<void>("setProperty", "offset", 3);
    EXPECT_EQ(3, prop->value().value().toInt());
  }

  // test error handling
  EXPECT_TRUE(client.setProperty("canard", 5).hasError());
  EXPECT_TRUE(client.setProperty("offset", "astring").hasError());
}


class Bar
{
public:
  void ping() { }
};

QI_REGISTER_OBJECT(Bar, ping)

TEST(QiService, RemoteServiceRegistrationAfterDisconnection)
{
  TestSessionPair p;

  // Create an object
  boost::shared_ptr<Bar> bar(new Bar());
  qi::AnyObject barAsObject = qi::AnyValue::from(bar).to<qi::AnyObject>();
  auto server = p.server();

  // Register the object with the provider, find it back from the client
  server->registerService("Bar", barAsObject);
  qi::AnyObject barAsRemoteService = p.client()->service("Bar").value();
  ASSERT_TRUE(barAsRemoteService);

  // Disconnect the provider, it should unregister any related services
  server->close();
  qiLogVerbose() << "close finished";
  qi::Future<void> fc = server->connect(test::url(*p.sd()));
  fc.wait(3000);
  if (fc.hasError())
    qiLogError() << fc.error();
  ASSERT_TRUE(fc.hasValue());
  qiLogVerbose() << "Connect finished";

  // Register the object again with the provider, find it back from the client
  server->listen("tcp://localhost:0").value();
  ASSERT_NO_THROW(server->registerService("Bar", barAsObject));
  ASSERT_EQ(qi::FutureState_FinishedWithValue,
            p.client()->waitForService("Bar").wait(qi::Seconds{ 3 }));
  ASSERT_TRUE(p.client()->service("Bar").value());
}

class DummyObject
{
public:
  DummyObject(qi::Promise<void> prom) : p(prom), a(0) { }
  ~DummyObject()
  {
      p.setValue(0);
  }
  qi::Promise<void> p;
  int a;
};

QI_REGISTER_OBJECT(DummyObject, a);

class ServiceThatServesObjects
{
public:

  qi::AnyObject getObject()
  {
      return qi::Object<DummyObject>(new DummyObject(prom));
  }

  qi::Promise<void> prom;
};

QI_REGISTER_OBJECT(ServiceThatServesObjects, getObject);

TEST(QiService, NetworkObjectsAreClosedWithTheSession)
{
  auto server = qi::makeSession();
  auto client = qi::makeSession();
  ServiceThatServesObjects *concreteService = new ServiceThatServesObjects;
  qi::Future<void> fut = concreteService->prom.future();

  server->listenStandalone(qi::Url("tcp://127.0.0.1:0"));
  server->registerService("service", qi::Object<ServiceThatServesObjects>(concreteService));
  client->connect(server->endpoints()[0]);

  ASSERT_TRUE(client->isConnected());
  qi::AnyObject service = client->service("service").value();
  qi::AnyObject obj = service.call<qi::AnyObject>("getObject");
  client->close();
  fut.wait();
  // if we reach here, the test is a success: the remote reference "client"
  // is gone so our object has been deleted.
}

class DoSomething
{
public:
  int ping(int i) { return i; }
};
QI_REGISTER_OBJECT(DoSomething, ping);

class CallDoSomethingInDtor
{
public:
  CallDoSomethingInDtor(const qi::SessionPtr& session)
    : _session(session)
  {
  }

  ~CallDoSomethingInDtor()
  {
    // Should be always work event on client session
    // ie: call Dtor object THEN close session and not the other way around
    if (_session->isConnected())
    {
      qiLogFatal() << "get doSomething service";
      _doSomething = _session->service("doSomething").value();
      qi::detail::printMetaObject(std::cout, _doSomething.metaObject());
      qiLogFatal() << "call doSomething.ping()";
      _doSomething.call<int>("ping", 12);
    }
  }

  void doNothing() {}

private:
  qi::AnyObject _doSomething;
  qi::SessionPtr _session;
};
QI_REGISTER_OBJECT(CallDoSomethingInDtor, doNothing);

TEST(QiService, CallRemoteServiceInsideDtorService)
{
  TestSessionPair p;

  auto ds = boost::make_shared<DoSomething>();
  p.server()->registerService("doSomething", qi::Object<DoSomething>(ds)).wait();

  auto callds = boost::make_shared<CallDoSomethingInDtor>(p.client());
  unsigned int idCallDS =
      p.server()->registerService("callDoSomethingInDtor", qi::Object<CallDoSomethingInDtor>(callds)).value();

  {
    EXPECT_NO_THROW(p.client()->service("doSomething").value());
    EXPECT_NO_THROW(p.client()->service("callDoSomethingInDtor").value());
  }

  // Have the client unregister the service
  // this should not deadlock
  p.client()->unregisterService(idCallDS).wait();
}


TEST(QiService, ExceptionFromPropertySetterSetsErrorOnFuture)
{
  using CustomException = std::exception;
  const int initialValue = 12;
  qi::Property<int> property{initialValue, qi::Property<int>::Getter{}, [](int&, const int&)->bool
  {
    throw CustomException{};
  }};

  const std::string serviceName{"Corine"};
  const std::string propertyName{"Ptitegoutte"};

  qi::DynamicObjectBuilder objectBuilder;
  objectBuilder.advertiseProperty(propertyName, &property);

  TestSessionPair sessions;
  sessions.server()->registerService(serviceName, objectBuilder.object());
  auto setting = sessions.client()->service(serviceName).value().setProperty(propertyName, 42);
  auto settingState = setting.waitFor(usualTimeout);
  ASSERT_EQ(qi::FutureState_FinishedWithError, settingState);
}

TEST(QiService, BlockingPropertySetterDoesNotBlockOtherCalls)
{
  std::promise<void> promise;
  qi::Property<int> property{qi::Property<int>::Getter{}, [&](int&, const int&)->bool
  {
    promise.get_future().wait();
    return false;
  }};

  const std::string serviceName{"Alain"};
  const std::string propertyName{"Alex"};
  const std::string methodName{"Terieur"};

  qi::DynamicObjectBuilder objectBuilder;
  objectBuilder.advertiseProperty(propertyName, &property);
  objectBuilder.advertiseMethod(methodName, []{});
  objectBuilder.setThreadingModel(qi::ObjectThreadingModel_MultiThread);

  TestSessionPair sessions;
  sessions.server()->registerService(serviceName, objectBuilder.object());

  auto remoteService = sessions.client()->service(serviceName).value();
  remoteService.setProperty(propertyName, 42).async();
  auto calling = remoteService.async<void>(methodName);
  auto callingState = calling.waitFor(usualTimeout);
  EXPECT_EQ(qi::FutureState_FinishedWithValue, callingState);
  promise.set_value();
}

////////////////////////////////////////////////////////////////////////////////
/// Optionals
////////////////////////////////////////////////////////////////////////////////
using boost::make_optional;

class OptPropService
{
public:
  struct Position
  {
    int x;
    int y;

    KA_GENERATE_FRIEND_REGULAR_OPS_2(Position, x, y)

    friend std::ostream& operator<<(std::ostream& os, const Position& pos)
    {
      return os << "Position(" << pos.x << ", " << pos.y << ")";
    }
  };

  qi::Property<boost::optional<std::string>> info;
  qi::Property<boost::optional<Position>> pos;
};
QI_TYPE_STRUCT(OptPropService::Position, x, y)
QI_REGISTER_OBJECT(OptPropService, info, pos)

namespace
{
  class QiServiceOptionalProperty : public ::testing::Test
  {
  protected:
    struct ObjectAssertingProxy
    {
      explicit ObjectAssertingProxy(qi::AnyObject& obj)
        : obj(obj)
      {}

      template <typename V>
      ::testing::AssertionResult set(const char* propName, V&& value)
      {
        const auto fut = obj.setProperty(propName, std::forward<V>(value));
        if (fut.hasError())
          return ::testing::AssertionFailure() << fut.error();
        return ::testing::AssertionSuccess();
      }

      template <typename T>
      ::testing::AssertionResult isSet(const char* propName, bool expected)
      {
        const auto fut = obj.property<boost::optional<T>>(propName);
        if (fut.hasError())
          return ::testing::AssertionFailure() << fut.error();
        const auto actual = static_cast<bool>(fut.value());
        if (actual != expected)
          return ::testing::AssertionFailure() << "value set state '" << std::boolalpha << actual
                                               << "' is not equal to '" << expected << "'";
        return ::testing::AssertionSuccess();
      }

      template <typename T>
      ::testing::AssertionResult valueEquals(const char* propName, T&& value = T())
      {
        const auto fut = obj.property<boost::optional<T>>(propName);
        if (fut.hasError())
          return ::testing::AssertionFailure() << fut.error();
        const auto futValue = fut.value();
        if (!futValue)
          return ::testing::AssertionFailure() << "optional is not set";
        if (*futValue != value)
          return ::testing::AssertionFailure() << "'" << futValue << "' is not equal to '" << value << "'";
        return ::testing::AssertionSuccess();
      }

      qi::AnyObject& obj;
    };

    QiServiceOptionalProperty()
      : serviceObject([=] {
        qi::ObjectTypeBuilder<OptPropService> builder;
        builder.advertiseProperty("info", &OptPropService::info);
        builder.advertiseProperty("pos", &OptPropService::pos);
        auto obj = builder.object(&optPropService, &qi::AnyObject::deleteGenericObjectOnly);
        session.server()->registerService("optpropservice", obj);
        return obj;
      }())
      , client(session.client()->service("optpropservice").value())
      , object(client)
    {
    }

    void setValues()
    {
      EXPECT_TRUE(object.set("info", make_optional<std::string>("pineapples")));
      EXPECT_TRUE(object.set("pos", make_optional(OptPropService::Position{-3242, 024})));
      ASSERT_TRUE(object.isSet<std::string>("info", true));
      ASSERT_TRUE(object.isSet<OptPropService::Position>("pos", true));
    }

    OptPropService optPropService;
    TestSessionPair session;
    qi::AnyObject serviceObject;
    qi::AnyObject client;
    ObjectAssertingProxy object;
  };
}

TEST_F(QiServiceOptionalProperty, IsUnsetByDefault)
{
  EXPECT_TRUE(object.isSet<std::string>("info", false));
  EXPECT_TRUE(object.isSet<OptPropService::Position>("pos", false));
}

TEST_F(QiServiceOptionalProperty, IsSetAfterSetting)
{
  ASSERT_TRUE(object.set("info", make_optional<std::string>("cookies")));
  ASSERT_TRUE(object.set("pos", make_optional(OptPropService::Position{37, 74})));

  EXPECT_TRUE(object.isSet<std::string>("info", true));
  EXPECT_TRUE(object.isSet<OptPropService::Position>("pos", true));

  EXPECT_TRUE(object.valueEquals<std::string>("info", "cookies"));
  EXPECT_TRUE(object.valueEquals<OptPropService::Position>("pos", OptPropService::Position{37, 74}));
}

TEST_F(QiServiceOptionalProperty, CanBeSetFromUnderlyingType)
{
  ASSERT_TRUE(object.set("info", std::string{"muffins"}));
  ASSERT_TRUE(object.set("pos", OptPropService::Position{42, 1337}));

  EXPECT_TRUE(object.isSet<std::string>("info", true));
  EXPECT_TRUE(object.isSet<OptPropService::Position>("pos", true));

  EXPECT_TRUE(object.valueEquals<std::string>("info", "muffins"));
  EXPECT_TRUE(object.valueEquals<OptPropService::Position>("pos", OptPropService::Position{42, 1337}));
}

TEST_F(QiServiceOptionalProperty, IsNotSetAfterSettingToNone)
{
  setValues();

  EXPECT_TRUE(object.set("info", boost::none));
  EXPECT_TRUE(object.set("pos", boost::none));
  EXPECT_TRUE(object.isSet<std::string>("info", false));
  EXPECT_TRUE(object.isSet<OptPropService::Position>("pos", false));
}

TEST_F(QiServiceOptionalProperty, IsNotSetAfterSettingVoidAnyValue)
{
  setValues();

  EXPECT_TRUE(object.set("info", qi::AnyValue::makeVoid()));
  EXPECT_TRUE(object.set("pos", qi::AnyValue::makeVoid()));
  EXPECT_TRUE(object.isSet<std::string>("info", false));
  EXPECT_TRUE(object.isSet<OptPropService::Position>("pos", false));
}

TEST_F(QiServiceOptionalProperty, CannotBeSetFromUnrelatedTypes)
{
  EXPECT_FALSE(object.set("info", 42));
  EXPECT_FALSE(object.set("pos", "abc"));
}
