/*
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <vector>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <qimessaging/session.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/gateway.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <testsession/testsessionpair.hpp>

static std::string reply(const std::string &msg)
{
  return msg;
}

/* For asynchronous things where no synchronisation mechanism
 * is possible, loop the check and wait a small delay,
 * instead of one big sleep that will slow us down
 *
 */
#define PERSIST_ASSERT(code, cond, msdelay)  \
do                                           \
{                                            \
  code;                                      \
  for(unsigned i=0; i<50 && !(cond); ++i)    \
  {                                          \
    qi::os::msleep(1 + msdelay / 50);        \
    code;                                    \
  }                                          \
  ASSERT_TRUE(cond);                         \
} while(0)

//check for server closed
//check for socket disconnected
//check for service unregistered
//check for service unregistered, then readded

TEST(QiService, RemoteObjectCacheServerClose)
{
  TestSessionPair p;

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  p.server()->registerService("serviceTest", obj);
  qi::Future<qi::ObjectPtr> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value()->call<std::string>("reply", "titi").value());

  p.server()->close();

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), 1000);
}


TEST(QiService, RemoteObjectCacheUnregister)
{
  TestSessionPair p;

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  unsigned int idx = p.server()->registerService("serviceTest", obj);
  qi::Future<qi::ObjectPtr> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value()->call<std::string>("reply", "titi").value());

  p.server()->unregisterService(idx);

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), 1000);

}


TEST(QiService, RemoteObjectCacheABAUnregister)
{
  TestSessionPair p;

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  unsigned int idx = p.server()->registerService("serviceTest", obj);
  qi::Future<qi::ObjectPtr> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value()->call<std::string>("reply", "titi").value());

  p.server()->unregisterService(idx);

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), 1000);

  unsigned int idx2 = p.server()->registerService("serviceTest", obj);
  //new service should not have a previoulsy registered ID
  EXPECT_NE(idx2, idx);

  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  qi::Future<std::string> fret = fut.value()->call<std::string>("reply", "titi");
  if (fret.hasError()) {
    std::cout << "Error returned:" << fret.error();
  }
  EXPECT_FALSE(fret.hasError());
  EXPECT_EQ(std::string("titi"), fret.value());
}


TEST(QiService, RemoteObjectCacheABANewServer)
{
  TestSessionPair p;
  qi::Session     ses;

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  qi::ObjectPtr obj(ob.object());

  unsigned int idx = p.server()->registerService("serviceTest", obj);
  qi::Future<qi::ObjectPtr> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  EXPECT_EQ(std::string("titi"), fut.value()->call<std::string>("reply", "titi").value());

  p.server()->close();

  PERSIST_ASSERT(fut = p.client()->service("serviceTest"), fut.hasError(), 1000);

  EXPECT_TRUE(ses.connect(p.client()->url().str()));
  ses.listen("tcp://0.0.0.0:0");
  unsigned int idx2 = ses.registerService("serviceTest", obj);
  //new service should not have a previoulsy registered ID
  EXPECT_NE(idx2, idx);

  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  qi::Future<std::string> fret = fut.value()->call<std::string>("reply", "titi");
  if (fret.hasError()) {
    std::cout << "Error returned:" << fret.error();
  }
  EXPECT_FALSE(fret.hasError());
  EXPECT_EQ(std::string("titi"), fret.value());
}


TEST(QiService, RemoteObjectNackTransactionWhenServerClosed)
{
  TestSessionPair p;

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("msleep", &qi::os::msleep);
  qi::ObjectPtr obj(ob.object());

  p.server()->registerService("serviceTest", obj);
  qi::Future<qi::ObjectPtr> fut;
  fut = p.client()->service("serviceTest");
  EXPECT_FALSE(fut.hasError());

  qi::Future<void> fret = fut.value()->call<void>("msleep", 2000);
  qi::Future<void> fclose = p.server()->close();
  fclose.wait(1000);
  EXPECT_TRUE(fclose.isReady());
  EXPECT_FALSE(fclose.hasError(1));

  fret.wait(1000);
  //once the server is close, the answer should be ready.
  EXPECT_TRUE(fret.isReady());
  //the service is closed, so it cant send an answer.
  EXPECT_TRUE(fret.hasError(1000));
}



int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  ::testing::InitGoogleTest(&argc, argv);
  TestMode::initTestMode(argc, argv);
  return RUN_ALL_TESTS();
}
