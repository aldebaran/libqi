/*
**
** Copyright (C) 2018 Softbank Robotics Europe
*/

#include <thread>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <testsession/testsessionpair.hpp>
#include <qi/testutils/testutils.hpp>
#include <gtest/gtest.h>

qiLogCategory("test");

namespace {

  struct NewObject
  {
    template<class ObjectType, class T>
    ObjectType make(qi::SessionPtr) const
    {
      return ObjectType(boost::make_shared<T>());
    }
  };



  template<class ObjectFactory>
  struct AsService
  {
    ObjectFactory factory;

    template<class ObjectType, class T>
    auto make(qi::SessionPtr session) const
      -> decltype(factory.template make<ObjectType, T>(session))
    {
      ObjectType object = factory.template make<ObjectType, T>(session);

      static std::atomic<int> next_id{ 0 };
      const auto serviceName = "ServiceObject_" + qi::os::to_string(next_id++);

      session->registerService(serviceName, object);
      auto serviceObject = session->service(serviceName).value();

      return serviceObject;
    }
  };

  template<class ObjectType, class T, class ObjectFactory>
  ObjectType makeObject(ObjectFactory& objectFactory, qi::SessionPtr session)
  {
    return objectFactory.template make<ObjectType, T>(session);
  }

  template<class ObjectFactory>
  class SendObjectIdentityFactoryType : public ::testing::Test
  {
  public:
    ObjectFactory factory;
  };

  template<class ObjectFactory>
  class SendObjectIdentityInterfaceProxyFactoryType
    : public SendObjectIdentityFactoryType<ObjectFactory>
  {
  };

  using FactoryTypes = ::testing::Types< NewObject
                                       , AsService<NewObject>
                                       >;

  TYPED_TEST_SUITE(SendObjectIdentityFactoryType, FactoryTypes);
  TYPED_TEST_SUITE(SendObjectIdentityInterfaceProxyFactoryType, FactoryTypes);

}


static int next_dummy_id = 0;
struct dummy_t
{
  int value = next_dummy_id++;
  int one() const
  {
    return value;
  }
  qi::AnyObject identity(qi::AnyObject o) { return o; }
};

QI_REGISTER_OBJECT(dummy_t, one, identity);

TEST(SendObjectIdentity, IdentityPreservedInServiceDirectory)
{
  TestSessionPair sessions;

  auto object = qi::AnyObject(boost::make_shared<dummy_t>());
  sessions.server()->registerService("MyObject", object);

  auto remoteObject = sessions.client()->service("MyObject").value();
  EXPECT_EQ(object.uid(), remoteObject.uid());
  EXPECT_EQ(object, remoteObject);
}

TEST(SendObjectIdentity, IdentityOfRemoteObjects)
{
  TestSessionPair p;
  p.server()->registerService("plop", boost::make_shared<dummy_t>());

  qi::DynamicObjectBuilder builder;
  auto o = builder.object();

  qi::AnyObject remotePlop = p.client()->service("plop").value();
  auto remoteObject = remotePlop.call<qi::AnyObject>("identity", o);
  EXPECT_EQ(o, remoteObject);

  qi::DynamicObjectBuilder builder2;
  auto o2 = builder2.object();
  auto remoteObject2 = remotePlop.call<qi::AnyObject>("identity", o2);
  EXPECT_EQ(o2, remoteObject2);
  EXPECT_NE(o2, remoteObject);
  EXPECT_NE(remoteObject2, remoteObject);
  EXPECT_NE(remoteObject2, o);
}

TYPED_TEST(SendObjectIdentityFactoryType, IdentityOfRemoteObjectsDifferentProcess)
{
  using namespace qi;

  const Url serviceUrl{"tcp://127.0.0.1:54321"};
  test::ScopedProcess _{path::findBin("remoteserviceowner"),
    {"--qi-standalone", "--qi-listen-url=" + serviceUrl.str()}
  };

  auto client = makeSession();
  client->connect(serviceUrl);
  AnyObject service = client->service("PingPongService").value();
  auto original = makeObject<AnyObject, dummy_t>(this->factory, client);

  service.call<void>("give", original);
  AnyObject copy0 = service.call<AnyObject>("take");
  EXPECT_EQ(copy0, original);

  service.call<void>("give", copy0);
  AnyObject copy1 = service.call<AnyObject>("take");
  ASSERT_EQ(copy1, copy0);
  ASSERT_EQ(copy1, original);

  ASSERT_EQ(copy1.call<int>("one"), copy0.call<int>("one"));
}


class ObjectStore
{
  qi::AnyObject obj;
public:
  qi::AnyObject get() const
  {
    return obj;
  }
  void set(qi::AnyObject o)
  {
    obj = o;
  }
};

QI_REGISTER_OBJECT(ObjectStore, get, set);

TYPED_TEST(SendObjectIdentityFactoryType, IdentityMaintainedBetweenSessions)
{
  TestSessionPair sessionPair;
  auto original_store = boost::make_shared<ObjectStore>();
  sessionPair.server()->registerService("store", original_store);

  auto object = makeObject<qi::AnyObject, dummy_t>(this->factory, sessionPair.server());
  original_store->set(object);

  auto store_from_server = sessionPair.server()->service("store").value();
  auto store_from_client = sessionPair.client()->service("store").value();

  qi::AnyObject object_from_server_1 = store_from_server.call<qi::AnyObject>("get");
  qi::AnyObject object_from_server_2 = store_from_server.call<qi::AnyObject>("get");

  qi::AnyObject object_from_client_1 = store_from_client.call<qi::AnyObject>("get");
  qi::AnyObject object_from_client_2 = store_from_client.call<qi::AnyObject>("get");

  EXPECT_EQ(object_from_server_1, object);
  EXPECT_EQ(object_from_server_2, object);
  EXPECT_EQ(object_from_server_1, object_from_server_2);

  EXPECT_EQ(object_from_client_1, object);
  EXPECT_EQ(object_from_client_2, object);
  EXPECT_EQ(object_from_client_1, object_from_client_2);

  EXPECT_EQ(object_from_server_1, object_from_client_1);
  EXPECT_EQ(object_from_server_1, object_from_client_2);
  EXPECT_EQ(object_from_server_2, object_from_client_1);
  EXPECT_EQ(object_from_server_2, object_from_client_2);

  qi::Session outterSession;
  outterSession.connect(sessionPair.endpointToServiceSource());
  qi::AnyObject store_from_outter = outterSession.service("store").value();
  qi::AnyObject object_from_outter_1 = store_from_outter.call<qi::AnyObject>("get");
  qi::AnyObject object_from_outter_2 = store_from_outter.call<qi::AnyObject>("get");

  EXPECT_EQ(object_from_outter_1, object);
  EXPECT_EQ(object_from_outter_2, object);
  EXPECT_EQ(object_from_outter_1, object_from_outter_2);

  EXPECT_EQ(object_from_outter_1, object_from_client_1);
  EXPECT_EQ(object_from_outter_1, object_from_client_2);
  EXPECT_EQ(object_from_outter_2, object_from_client_1);
  EXPECT_EQ(object_from_outter_2, object_from_client_2);

  EXPECT_EQ(object_from_outter_1, object_from_server_1);
  EXPECT_EQ(object_from_outter_1, object_from_server_2);
  EXPECT_EQ(object_from_outter_2, object_from_server_1);
  EXPECT_EQ(object_from_outter_2, object_from_server_2);

}

TYPED_TEST(SendObjectIdentityFactoryType, IdentityMaintainedBetweenSessionsWithRemote)
{
  using namespace qi;

  const Url serviceUrl{ "tcp://127.0.0.1:54321" };
  test::ScopedProcess _{ path::findBin("remoteserviceowner"),
  { "--qi-standalone", "--qi-listen-url=" + serviceUrl.str() }
  };

  auto client = makeSession();
  client->connect(serviceUrl);
  AnyObject store_from_client = client->service("PingPongService").value();

  auto object = makeObject<AnyObject, dummy_t>(this->factory, client);
  store_from_client.call<void>("give", object);

  AnyObject object_from_client_1 = store_from_client.call<AnyObject>("take");
  AnyObject object_from_client_2 = store_from_client.call<AnyObject>("take");

  EXPECT_EQ(object_from_client_1, object);
  EXPECT_EQ(object_from_client_2, object);
  EXPECT_EQ(object_from_client_1, object_from_client_2);

  Session outterSession;
  outterSession.connect(serviceUrl);
  AnyObject store_from_outter = outterSession.service("PingPongService").value();
  AnyObject object_from_outter_1 = store_from_outter.call<AnyObject>("take");
  AnyObject object_from_outter_2 = store_from_outter.call<AnyObject>("take");

  EXPECT_EQ(object_from_outter_1, object);
  EXPECT_EQ(object_from_outter_2, object);
  EXPECT_EQ(object_from_outter_1, object_from_outter_2);

  EXPECT_EQ(object_from_outter_1, object_from_client_1);
  EXPECT_EQ(object_from_outter_1, object_from_client_2);
  EXPECT_EQ(object_from_outter_2, object_from_client_1);
  EXPECT_EQ(object_from_outter_2, object_from_client_2);

}


TYPED_TEST(SendObjectIdentityFactoryType, IdentityOfRemoteObjectsMoreIndirections)
{
  TestSessionPair pairA;
  auto originalObject = makeObject<qi::AnyObject, dummy_t>(this->factory, pairA.server());
  pairA.server()->registerService("serviceA", boost::make_shared<ObjectStore>());
  qi::AnyObject clientA = pairA.client()->service("serviceA").value();
  clientA.call<void>("set", originalObject);
  qi::AnyObject objA = clientA.call<qi::AnyObject>("get");
  EXPECT_EQ(originalObject, objA);

  TestSessionPair pairB;
  pairB.server()->registerService("serviceB", boost::make_shared<ObjectStore>());
  qi::AnyObject clientB = pairB.client()->service("serviceB").value();
  clientB.call<void>("set", objA);
  qi::AnyObject objB = clientB.call<qi::AnyObject>("get");
  EXPECT_EQ(originalObject, objB);

  TestSessionPair pairC;
  pairC.server()->registerService("serviceC", boost::make_shared<ObjectStore>());
  qi::AnyObject clientC = pairC.client()->service("serviceC").value();
  clientC.call<void>("set", objB);
  qi::AnyObject objC = clientC.call<qi::AnyObject>("get");
  EXPECT_EQ(originalObject, objC);
  EXPECT_EQ(objA, objC);
  EXPECT_EQ(objB, objC);
}

////////////////////////////////////////////////////////////////////////////
// The following tests check that object identity is maintained when
// using the interface/proxy/impl system of libqi.
// An interface type is always registered with an associated proxy type.
// The proxy type will be instantiated when receiving an object of the
// interface type. The real implementation can be any type compatible
// with the interface type.
// Here we want to make sure that the uid of the real implementation
// object is propagated to all its proxies in all kinds of situations
// so that it is always possible to identify if a proxy is actually representing
// the same implementation object.
//////

struct SomeInterface
{
  virtual ~SomeInterface() = default;
  virtual int get() const = 0;
};

QI_REGISTER_OBJECT(SomeInterface, get)

struct SomeInterfaceProxy : SomeInterface, public qi::Proxy
{
  SomeInterfaceProxy(qi::AnyObject o)
    : qi::Proxy(o) {}

  int get() const override { return asObject().call<int>("get"); }
};
QI_REGISTER_PROXY_INTERFACE(SomeInterfaceProxy, SomeInterface);

static std::atomic<int> nextSomeInterfaceId{ 0 };

class SomeInterfaceImpl : public SomeInterface
{
  int id = nextSomeInterfaceId++;
public:
  int get() const override { return id; }
};
QI_REGISTER_OBJECT(SomeInterfaceImpl, get)

// See QI_REGISTER_IMPLEMENTATION_H for details explaining why we need to build all this.
template<class Interface, class SharedT>
class SomeInterfaceImplWrapper : public Interface
{
  SharedT impl;
public:
  explicit SomeInterfaceImplWrapper(SharedT impl) : impl(impl) {}
  int get() const override { return impl->get(); }
};

template<class SharedT>
using SomeInterfaceLocalAsync = SomeInterfaceImplWrapper<SomeInterface, SharedT>;
template<class SharedT>
using SomeInterfaceLocalSync = SomeInterfaceImplWrapper<SomeInterface, SharedT>;

class SomeInterfaceImplNoInheritance
{
  int id = nextSomeInterfaceId++;
public:
  int get() const { return id; }
};
QI_REGISTER_OBJECT(SomeInterfaceImplNoInheritance, get)
QI_REGISTER_IMPLEMENTATION_H(SomeInterface, SomeInterfaceImplNoInheritance)


TEST(SendObjectIdentityInterfaceProxy, IdentityDependsOnObjectAddressWithAnyObject)
{
  using namespace qi;
  auto realObject = boost::make_shared<SomeInterfaceImpl>();
  const ObjectUid uid = os::ptrUid(realObject.get());
  Object<SomeInterface> a{ AnyObject{ realObject } };
  Object<SomeInterface> b{ AnyObject{ realObject } };

  EXPECT_EQ(uid, a.uid());
  EXPECT_EQ(a, b);
  EXPECT_EQ(a->get(), b->get());
}

TEST(SendObjectIdentityInterfaceProxy, IdentityDependsOnObjectAddressWithObjectT)
{
  using namespace qi;
  auto realObject = boost::make_shared<SomeInterfaceImpl>();
  const ObjectUid uid = os::ptrUid(realObject.get());
  Object<SomeInterface> a{ realObject };
  Object<SomeInterface> b{ realObject };

  EXPECT_EQ(uid, a.uid());
  EXPECT_EQ(a, b);
  EXPECT_EQ(a->get(), b->get());
}



TEST(SendObjectIdentityInterfaceProxy, IdentityDependsOnObjectAddressWithAnyObjectNoInheritance)
{
  using namespace qi;
  auto realObject = boost::make_shared<SomeInterfaceImplNoInheritance>();
  const ObjectUid uid = os::ptrUid(realObject.get());
  // Object constructor implicitely instanciates SomeInterfaceLocalSync<shared_ptr<SomeInterfaceImplNoInheritance>>
  Object<SomeInterface> a{ AnyObject{ realObject } };
  Object<SomeInterface> b{ AnyObject{ realObject } };

  EXPECT_EQ(uid, a.uid());
  EXPECT_EQ(a, b);
  EXPECT_EQ(a->get(), b->get());
}

TEST(SendObjectIdentityInterfaceProxy, IdentityDependsOnObjectAddressWithObjectTNoInheritance)
{
  using namespace qi;
  auto realObject = boost::make_shared<SomeInterfaceImplNoInheritance>();
  const ObjectUid uid = os::ptrUid(realObject.get());
  // Object constructor implicitely instanciates SomeInterfaceLocalSync<shared_ptr<SomeInterfaceImplNoInheritance>>
  Object<SomeInterface> a{ realObject };
  Object<SomeInterface> b{ realObject };

  EXPECT_EQ(uid, a.uid());
  EXPECT_EQ(a, b);
  EXPECT_EQ(a->get(), b->get());
}



TYPED_TEST(SendObjectIdentityInterfaceProxyFactoryType, IdentityIsMaintainedWhenSentToRemoteAnyObjectStoreRetrievingAnyObject)
{
  using namespace qi;

  TestSessionPair sessions;

  auto original = makeObject<qi::AnyObject, SomeInterfaceImpl>(this->factory, sessions.server());
  sessions.server()->registerService("Store", boost::make_shared<ObjectStore>());
  AnyObject store = sessions.client()->service("Store").value();
  store.call<void>("set", original);

  Object<SomeInterface> objectA = store.call<AnyObject>("get");
  EXPECT_EQ(original, objectA) << "original uid: {" << original.uid() << "}; objectA uid: {" << objectA.uid() << "};";

}

TYPED_TEST(SendObjectIdentityInterfaceProxyFactoryType, IdentityIsMaintainedWhenSentToRemoteAnyObjectStoreRetrievingObjectT)
{
  using namespace qi;

  TestSessionPair sessions;

  auto original = makeObject<qi::AnyObject, SomeInterfaceImpl>(this->factory, sessions.server());
  sessions.server()->registerService("Store", boost::make_shared<ObjectStore>());
  AnyObject store = sessions.client()->service("Store").value();
  store.call<void>("set", original);

  Object<SomeInterface> objectA = store.call<Object<SomeInterface>>("get");
  EXPECT_EQ(original, objectA) << "original uid: {" << original.uid() <<"}; vs objectA uid: {" << objectA.uid() << "};";

}

struct SomeStore
{
  virtual ~SomeStore() = default;
  virtual qi::Object<SomeInterface> get() const = 0;
  virtual void set(qi::Object<SomeInterface> o) = 0;
};
QI_REGISTER_OBJECT(SomeStore, get, set);

class SomeStoreProxy : public SomeStore, public qi::Proxy
{
public:
  SomeStoreProxy(qi::AnyObject o)
    : qi::Proxy(o) {}

  qi::Object<SomeInterface> get() const override
  {
    return asObject().call<qi::Object<SomeInterface>>("get");
  }
  void set(qi::Object<SomeInterface> o) override
  {
    return asObject().call<void>("set", o);
  }
};
QI_REGISTER_PROXY_INTERFACE(SomeStoreProxy, SomeStore);

class SomeStoreImpl : SomeStore
{
  qi::Object<SomeInterface> obj;
public:
  qi::Object<SomeInterface> get() const override
  {
    return obj;
  }
  void set(qi::Object<SomeInterface> o) override
  {
    obj = o;
  }
};
QI_REGISTER_OBJECT(SomeStoreImpl, get, set);

namespace
{
  template<class ObjectFactory>
  class SomeInterfaceFactoryType
    : public SendObjectIdentityFactoryType<ObjectFactory>
  {
  };
  TYPED_TEST_SUITE(SomeInterfaceFactoryType, FactoryTypes);
}

TYPED_TEST(SomeInterfaceFactoryType, IdentityIsMaintainedWhenSentToInterfaceSpecializedStoreRetrievingAnyObject)
{
  using namespace qi;

  TestSessionPair sessions;

  auto original = makeObject<Object<SomeInterface>, SomeInterfaceImpl>(this->factory, sessions.server());
  sessions.server()->registerService("Store", boost::make_shared<SomeStoreImpl>());
  Object<SomeStore> store = sessions.client()->service("Store");
  store->set(original);

  Object<SomeInterface> objectA = store->get();
  EXPECT_EQ(original, objectA) << "original uid: {" << original.uid() << "}; vs objectA uid: {" << objectA.uid() << "};";

}

TYPED_TEST(SomeInterfaceFactoryType, IdentityIsMaintainedWhenSentToRemoteProcessAnyObjectStoreRetrievingAnyObject)
{
  using namespace qi;

  const Url serviceUrl{ "tcp://127.0.0.1:54321" };
  test::ScopedProcess _{ path::findBin("remoteserviceowner"),
  { "--qi-standalone", "--qi-listen-url=" + serviceUrl.str() }
  };

  auto client = makeSession();
  client->connect(serviceUrl);
  AnyObject service = client->service("PingPongService").value();
  auto original = makeObject<Object<SomeInterface>, SomeInterfaceImpl>(this->factory, client);

  service.call<void>("give", original);
  AnyObject copy0 = service.call<AnyObject>("take");
  EXPECT_EQ(copy0, original) << "copy0 uid: {" << copy0.uid() << "}; vs original uid: {" << original.uid() << "};";

  service.call<void>("give", copy0);
  AnyObject copy1 = service.call<AnyObject>("take");
  EXPECT_EQ(copy1, copy0) << "copy1 uid: {" << copy1.uid() << "}; vs copy0 uid: {" << copy0.uid() << "};";
  EXPECT_EQ(copy1, original) << "copy1 uid: {" << copy1.uid() << "}; vs original uid: {" << original.uid() << "};";
}



TYPED_TEST(SomeInterfaceFactoryType, IdentityIsMaintainedWhenSentToRemoteProcessAnyObjectStoreRetrievingObjectT)
{
  using namespace qi;

  const Url serviceUrl{ "tcp://127.0.0.1:54321" };
  test::ScopedProcess _{ path::findBin("remoteserviceowner"),
  { "--qi-standalone", "--qi-listen-url=" + serviceUrl.str() }
  };

  auto client = makeSession();
  client->connect(serviceUrl);
  AnyObject service = client->service("PingPongService").value();
  auto original = makeObject<Object<SomeInterface>, SomeInterfaceImpl>(this->factory, client);

  service.call<void>("give", original);
  Object<SomeInterface> copy0 = service.call<Object<SomeInterface>>("take");
  EXPECT_EQ(copy0, original) << "copy0 uid: {" << copy0.uid() << "}; vs original uid: {" << original.uid() << "};";

  service.call<void>("give", copy0);
  Object<SomeInterface> copy1 = service.call<Object<SomeInterface>>("take");
  EXPECT_EQ(copy1, copy0) << "copy1 uid: {" << copy1.uid() << "}; vs copy0 uid: {" << copy0.uid() << "};";
  EXPECT_EQ(copy1, original) << "copy1 uid: {" << copy1.uid() << "}; vs original uid: {" << original.uid() << "};";
}

////////
// End of tests about object identification with interface/proxy/impl system.
////////////////////////////////////////////////////////////////////////////////
