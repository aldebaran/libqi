/*
** Author(s):
**  - Guillaume OREAL <goreal@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <future>
#include <vector>
#include <string>
#include <gtest/gtest.h>
#include <qi/session.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <testsession/testsessionpair.hpp>
#include <qi/testutils/testutils.hpp>
#include "objectio.hpp"

qiLogCategory("qi.test_call_on_close_session");

using namespace qi;
using namespace test;

namespace
{
  static const auto iterations = 100;
  static const auto dummyServiceName = "serviceTest";

  std::string waitAndReply(qi::Promise<void> startPromise, const std::string& msg)
  {
    startPromise.future().wait();
    return msg;
  }

  AnyObject dummyDynamicObject()
  {
    DynamicObjectBuilder ob;
    ob.advertiseMethod("waitAndReply", &waitAndReply);
    return ob.object();
  }
}

TEST(TestCallOnCloseSession, CloseSessionBeforeCallEnds)
{
  auto obj = dummyDynamicObject();

  for (int i = 0; i < iterations; i++)
  {
    TestSessionPair sessionPair;
    auto& server = *sessionPair.server();
    auto& client = *sessionPair.client();

    ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, obj)));

    AnyObject myService;
    ASSERT_TRUE(finishesWithValue(client.service(dummyServiceName), willAssignValue(myService)));

    Promise<void> startReplyPromise;
    auto f = std::async(std::launch::async,
                        [&] { myService.call<std::string>("waitAndReply", "ok", startReplyPromise); });
    ASSERT_TRUE(finishesWithValue(server.close()));
    startReplyPromise.setValue(nullptr);
    EXPECT_ANY_THROW(f.get());
  }
}

TEST(TestCallOnCloseSession, GettingServiceWhileDisconnecting)
{
  auto object = dummyDynamicObject();

  for(int i = 0; i < iterations; ++i)
  {
    TestSessionPair sessionPair;
    auto& server = *sessionPair.server();
    auto& client = *sessionPair.client();

    ASSERT_TRUE(finishesWithValue(server.registerService(dummyServiceName, object)));

    auto closing = client.close().async();
    try
    {
      AnyObject remoteObject = client.service(dummyServiceName).value();
      ASSERT_TRUE(remoteObject); // if future did not throw at this point, then object must be valid
    }
    catch(const FutureException& e)
    {
      qiLogDebug() << "Got expected error: " << e.what();
      SUCCEED() << "Got expected error: " << e.what();
    }
    catch(const std::exception& e)
    {
      qiLogDebug() << "Got standard error: " << e.what();
      FAIL() << "Got standard error: " << e.what();
    }
    ASSERT_TRUE(finishesWithValue(closing));
  }
}
