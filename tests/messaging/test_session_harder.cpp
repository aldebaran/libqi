/*
 ** Author(s):
 **  - Cedric GESTES <cgestes@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <qi/session.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <qi/type/typeinterface.hpp>
#include <boost/thread.hpp>
#include <testsession/testsessionpair.hpp>

qiLogCategory("test");

struct MyStruct {
  int i;
  int j;
  std::string titi;
};

qi::Seconds callTimeout{5};
std::chrono::seconds unitTimeout{15};

QI_TYPE_STRUCT(MyStruct, i, j, titi);

static std::string reply(const std::string &msg)
{
  return msg;
}

static MyStruct reply2(const MyStruct &mystruct) {
  return mystruct;
}

qi::AnyObject newObject() {
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  ob.advertiseMethod("reply2", &reply2);
  return ob.object();
}



namespace {

  class ScopedThread
  {
    boost::thread _thread;
  public:

    template< class... Args >
    explicit ScopedThread(Args&&... args)
      : _thread{ std::forward<Args>(args)... }
    {
    }

    ~ScopedThread()
    {
      _thread.interrupt();
      _thread.join();
    }

  };

#define ASSERT_PROPAGATE_FAILURE( expr__ ) { if( expr__ ); else \
    { \
      throw ::testing::AssertionFailure() << '\n' << __FILE__ << '(' << __LINE__ << "): Assertion failed: " << #expr__ << '\n'; \
    } \
  }


}

void alternateModule(qi::SessionPtr session) {
  while (!boost::this_thread::interruption_requested())
  {
    boost::this_thread::interruption_point();

    qi::AnyObject obj = newObject();
    qi::Future<unsigned int> fut = session->registerService("TestToto", obj);
    if (fut.hasError()) {
      std::cout << "Error registering service: " << fut.error() << std::endl;
      continue;
    }
    std::cout << "Service TestToto registered" << std::endl;
    qi::os::msleep(50);
    qi::Future<void> futun = session->unregisterService(fut.value());
    if (futun.hasError()) {
      std::cout << "Error unregistering service: " << futun.error() << std::endl;
      continue;
    }
    std::cout << "Service TestToto unregistered" << std::endl;
  }
}

template <typename FunctionType>
void repeatedlyCallServiceMaybeDying(
    TestSessionPair& p, int nofAttempts, std::chrono::milliseconds timeout, FunctionType f)
{
  const auto endTime = std::chrono::steady_clock::now() + timeout;
  auto server = p.server();
  ScopedThread worker{ [server] { alternateModule(server); } };
  while (nofAttempts && (endTime > std::chrono::steady_clock::now()))
  {
    --nofAttempts;
    try
    {
      f();
    }
    catch(const std::exception& e)
    {
      std::cout << "Call exception: " << e.what() << std::endl;
    }
    catch (const ::testing::AssertionResult& assertFailure)
    {
      ASSERT_TRUE(assertFailure);
    }
  }
}

TEST(QiSession, RegisterUnregisterTwoSession)
{
  int a = 1000;
  if (TestMode::getTestMode() == TestMode::Mode_SSL)
  {
    a /= 100;
  }
  TestSessionPair p;
  repeatedlyCallServiceMaybeDying(p, a, unitTimeout, [&]
  {
    qi::Future<qi::AnyObject> fut = p.client()->service("TestToto");

    ASSERT_PROPAGATE_FAILURE(fut.waitFor(callTimeout) != qi::FutureState_Running);

    if (fut.hasError(0))
    {
      std::cout << "Call error:" << fut.error() << std::endl;
    }
    else
    {
      fut.value().call<std::string>("reply", "plif");
    }
  });
}

TEST(QiSession, RegisterUnregisterSameSession)
{
  int a = 1000;
  TestSessionPair p;
  repeatedlyCallServiceMaybeDying(p, a, unitTimeout, [&]
  {
    qi::Future<qi::AnyObject> fut = p.server()->service("TestToto");

    ASSERT_PROPAGATE_FAILURE(fut.waitFor(callTimeout) != qi::FutureState_Running);

    if (fut.hasError(0))
    {
      std::cout << "Call error:" << fut.error() << std::endl;
    }
    else
    {
      fut.value().call<std::string>("reply", "plif");
    }
  });
}

TEST(QiSession, RegisterUnregisterTwoSessionStruct)
{
  int a = 1000;
  if (TestMode::getTestMode() == TestMode::Mode_SSL)
  {
    a /= 100;
  }
  TestSessionPair p;
  repeatedlyCallServiceMaybeDying(p, a, unitTimeout, [&]
  {
    qi::Future<qi::AnyObject> fut = p.client()->service("TestToto");

    ASSERT_PROPAGATE_FAILURE(fut.waitFor(callTimeout) != qi::FutureState_Running);

    if (fut.hasError(0))
    {
      std::cout << "Call error:" << fut.error() << std::endl;
    }
    else
    {
      MyStruct ms;
      ms.i = 32;
      ms.j = 42;
      ms.titi = "tutu";
      qi::Future<MyStruct> ret = fut.value().async<MyStruct>("reply2", ms);
      ASSERT_PROPAGATE_FAILURE(ret.waitFor(callTimeout) != qi::FutureState_Running);
      if (ret.hasError(0))
      {
        std::cout << "returned an error:" << fut.error() << std::endl;
      }
      else
      {
        ASSERT_PROPAGATE_FAILURE(ms.i == ret.value().i);
        ASSERT_PROPAGATE_FAILURE(ms.j == ret.value().j);
        ASSERT_PROPAGATE_FAILURE(ms.titi == ret.value().titi);
      }
    }

  });
}

TEST(QiSession, RegisterUnregisterSameSessionStruct)
{
  int a = 1000;
  if (TestMode::getTestMode() == TestMode::Mode_SSL)
  {
    a /= 100;
  }
  TestSessionPair p;
  repeatedlyCallServiceMaybeDying(p, a, unitTimeout, [&]
  {
    qi::Future<qi::AnyObject> fut = p.server()->service("TestToto");
    if (fut.hasError())
    {
      std::cout << "Call error:" << fut.error() << std::endl;
    }
    else
    {
      MyStruct ms;
      ms.i = 32;
      ms.j = 42;
      ms.titi = "tutu";
      qi::Future<MyStruct> ret = fut.value().async<MyStruct>("reply2", ms);
      ASSERT_PROPAGATE_FAILURE(ret.waitFor(callTimeout) != qi::FutureState_Running);
      if (ret.hasError(0))
      {
        std::cout << "returned an error:" << fut.error() << std::endl;
      }
      else
      {
        ASSERT_PROPAGATE_FAILURE(ms.i == ret.value().i);
        ASSERT_PROPAGATE_FAILURE(ms.j == ret.value().j);
        ASSERT_PROPAGATE_FAILURE(ms.titi == ret.value().titi);
      }
    }
  });
}

TEST(QiSession, ConnectToMultipleConstellation)
{
  TestSessionPair constellation1;
  TestSessionPair constellation2;
  TestSessionPair constellation3;
  qi::Session     traveler;

  qi::AnyObject obj = newObject();
  constellation1.server()->registerService("test1", obj);
  constellation2.server()->registerService("test2", obj);
  constellation3.server()->registerService("test3", obj);

  qi::Future<void> f;
  f = traveler.connect(constellation1.serviceDirectoryEndpoints()[0].str());
  f.waitFor(callTimeout);
  ASSERT_TRUE(!f.hasError());
  qi::AnyObject proxy = constellation1.server()->service("test1");
  std::string res = proxy.call<std::string>("reply", "plaf");
  ASSERT_TRUE(res.compare("plaf") == 0);
  traveler.close();

  f = traveler.connect(constellation2.serviceDirectoryEndpoints()[0].str());
  f.waitFor(callTimeout);
  ASSERT_TRUE(!f.hasError());
  proxy = constellation2.server()->service("test2");
  ASSERT_TRUE(!!proxy);
  res = proxy.call<std::string>("reply", "plaf");
  ASSERT_TRUE(res.compare("plaf") == 0);
  traveler.close();

  f = traveler.connect(constellation3.serviceDirectoryEndpoints()[0].str());
  f.waitFor(callTimeout);
  if (f.hasError())
    qiLogError() << f.error();
  ASSERT_TRUE(!f.hasError());
  proxy = constellation3.server()->service("test3");
  res = proxy.call<std::string>("reply", "plaf");
  ASSERT_TRUE(res.compare("plaf") == 0);
  traveler.close();
}
