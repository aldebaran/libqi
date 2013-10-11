#include <vector>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <qimessaging/session.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qitype/dynamicobject.hpp>
#include <qitype/objecttypebuilder.hpp>
#include <qimessaging/gateway.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <testsession/testsessionpair.hpp>

#include <qimessaging/autoservice.hpp>
#include <stdexcept>

qiLogCategory("test");

class Pong
{
public:
  Pong()
  {
    i = 42;
  }

  void ping()
  {
  }

  int incr()
  {
    return i++;
  }

public:
  int i;
};
QI_REGISTER_OBJECT(Pong, ping, incr);

class PongProxy : public qi::Proxy
{
public:
  PongProxy(qi::AnyObject o)
    : qi::Proxy(o)
  {
  }

  PongProxy()
  {
  }

  qi::FutureSync<void> ping()
  {
    return asObject().call<void>("ping");
  }

  qi::FutureSync<int> incr()
  {
    return asObject().call<int>("incr");
  }
};
QI_TYPE_PROXY(PongProxy)

TEST(QiAutoService, SimpleUsage)
{
  TestSessionPair pair;

  boost::shared_ptr<Pong> pong(new Pong());
  qi::AnyObject pongAsObject = qi::AnyValue::from(pong).to<qi::AnyObject>();
  qi::Future<unsigned int> fut = pair.server()->registerService("Ping pong", pongAsObject);
  fut.wait();

  qi::AutoService<PongProxy> as("Ping pong", *(pair.client()));
  as.waitForReady().wait();

  EXPECT_EQ(42, as->incr().value());
}

TEST(QiAutoService, MultiUsage)
{
  TestSessionPair pair;

  boost::shared_ptr<Pong> pong(new Pong());
  qi::AnyObject pongAsObject = qi::AnyValue::from(pong).to<qi::AnyObject>();
  qi::Future<unsigned int> fut = pair.server()->registerService("Ping pong", pongAsObject);
  fut.wait();

  qi::AutoService<PongProxy> as("Ping pong", *(pair.client()));
  as.waitForReady().wait();

  EXPECT_EQ(42, as->incr().value());
  EXPECT_EQ(43, as->incr().value());
}

TEST(QiAutoService, AutoReConnect)
{
  TestSessionPair pair;

  boost::shared_ptr<Pong> pong(new Pong());
  qi::AnyObject pongAsObject = qi::AnyValue::from(pong).to<qi::AnyObject>();
  qi::Future<unsigned int> fut = pair.server()->registerService("Ping pong", pongAsObject);
  fut.wait();

  qi::AutoService<PongProxy> as("Ping pong", *(pair.client()));
  as.waitForReady().wait();

  EXPECT_EQ(42, as->incr());

  pair.server()->unregisterService(fut.value());
  qi::Future<unsigned int> fut2 = pair.server()->registerService("Ping pong", pongAsObject);
  fut2.wait();

  while(true)
  {
    try
    {
      EXPECT_EQ(43, as->incr().value());
      break;
    }
    catch (std::runtime_error e)
    {
      std::cout << e.what() << std::endl;
    }
  }
}

TestSessionPair *trash = 0;

void trollRegister()
{
  boost::shared_ptr<Pong> pong(new Pong());
  qi::AnyObject pongAsObject = qi::AnyValue::from(pong).to<qi::AnyObject>();
  unsigned int j = trash->server()->registerService("Ping pong", pongAsObject).value();

  qi::os::msleep(200);
  for (int i = 0; i < 200; i++)
  {
    trash->server()->unregisterService(j).value();
    qi::os::msleep(qi::os::ustime() % 50);
    j = trash->server()->registerService("Ping pong", pongAsObject).value();
    qi::os::msleep(qi::os::ustime() % 50);
  }
}

void testSpam()
{
  qi::AutoService<PongProxy> as("Ping pong", *(trash->client()));
  as.waitForReady().wait();

  for (unsigned long long int i = 0; i < 100 ; i++)
  {
    try
    {
      qi::os::msleep(10 + qi::os::ustime() % 40);
      as->ping();
    }
    catch(std::runtime_error e)
    {
      i--;
    }
  }
}

TEST(QiAutoService, CompilingOperatorStar)
{
  TestSessionPair pair;

  boost::shared_ptr<Pong> pong(new Pong());
  qi::AnyObject pongAsObject = qi::AnyValue::from(pong).to<qi::AnyObject>();
  qi::Future<unsigned int> fut = pair.server()->registerService("Ping pong", pongAsObject);
  fut.wait();

  qi::AutoService<PongProxy> as("Ping pong", *(pair.client()));
  as.waitForReady().wait();

  EXPECT_EQ(42, ((*as).incr().value()));
}

TEST(QiAutoService, ThreadSafe)
{
  /*
   * Simulates a service that feels funny to infinitley register/unregister himself
   * Meanwhile three clients try to call the ping() method.
   */

  trash = new TestSessionPair();
  boost::thread_group group;
  std::ostringstream strConverter;

  group.create_thread(&trollRegister);
  qi::os::msleep(100);

  for (int i = 0; i < 10; i++)
    group.create_thread(testSpam);

  group.join_all();
  delete trash;
  EXPECT_TRUE(true); // or dead
}

void setTo42(int* n)
{
  *n = 42;
}

TEST(QiAutoService, Signals)
{
  int n = 0;
  TestSessionPair pair;

  boost::shared_ptr<Pong> pong(new Pong());
  qi::AnyObject pongAsObject = qi::AnyValue::from(pong).to<qi::AnyObject>();
  qi::Future<unsigned int> fut = pair.server()->registerService("Ping pong", pongAsObject);
  fut.wait();

  qi::AutoService<PongProxy> as("Ping pong", *(pair.client()));
  as.serviceRemoved.connect(boost::bind<void>(&setTo42, &n));
  pair.server()->unregisterService(fut.value()).wait();

  qi::os::msleep(100);
  EXPECT_EQ(42, n);
}

TEST(QiAutoService, IsReadyMethod)
{
  int n = 0;
  TestSessionPair pair;

  boost::shared_ptr<Pong> pong(new Pong());
  qi::AnyObject pongAsObject = qi::AnyValue::from(pong).to<qi::AnyObject>();
  qi::Future<unsigned int> fut = pair.server()->registerService("Ping pong", pongAsObject);
  fut.wait();

  qi::AutoService<PongProxy> as("Ping pong", *(pair.client()));
  qi::Future<void> fut2 = as.waitForReady();

  fut2.connect(&setTo42, &n);
  fut2.wait();
  qi::os::msleep(100);

  EXPECT_EQ(42, n);
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  TestMode::initTestMode(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
