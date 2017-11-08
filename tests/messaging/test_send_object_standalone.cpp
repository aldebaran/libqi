#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include <qi/log.hpp>
#include <qi/session.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/testutils/testutils.hpp>

qiLogCategory("test.send_object_standalone");

struct InterestingObject
{
  bool doStuff()
  {
    qiLogInfo() << "Done";
    return true;
  }
};

QI_REGISTER_OBJECT(InterestingObject, doStuff)

struct MiddleMan
{
  bool callOnArgument(qi::AnyObject o, const std::string &method)
  {
    return o.call<bool>(method);
  }
};

QI_REGISTER_OBJECT(MiddleMan, callOnArgument)

TEST(SendObjectToStandalone, MultiProcessPingPong_CallArgumentMethod)
{
  static const std::chrono::milliseconds sleepDuration{ 50 };
  using test::ScopedProcess;
  // Start a service directory in a separate process.
  const std::string sdPath = qi::path::findBin("simplesd");
  ScopedProcess sd {sdPath, {"--qi-listen-url=tcp://127.0.0.1:54321",
                             "--qi-standalone"}};

  auto client = qi::makeSession();
  for (int i = 0; i < 20; ++i)
  {
    std::this_thread::sleep_for(sleepDuration);
    try
    {
      client->connect("tcp://127.0.0.1:54321");
      break;
    }
    catch (const std::exception& e)
    {
      std::cout << "Service Directory is not ready yet (" << e.what() << ")" << std::endl;
    }
  }
  ASSERT_TRUE(client->isConnected());

  // Register a service in another process.
  const std::string remoteServiceOwnerPath =
      qi::path::findBin("remoteserviceowner");
  ScopedProcess remoteServiceOwner{
    remoteServiceOwnerPath, {"--qi-url=tcp://127.0.0.1:54321"}};

  auto session = qi::makeSession();
  session->connect("tcp://127.0.0.1:54321");
  session->waitForService("PingPongService").wait(5000);
  qi::AnyObject serviceProxy = session->service("PingPongService").value();
  serviceProxy.call<void>("give", boost::make_shared<MiddleMan>());
  qi::AnyObject middleman = serviceProxy.call<qi::AnyObject>("take");
  qi::AnyObject precious = boost::make_shared<InterestingObject>();
  for (auto i = 0; i < 5; ++i)
  {
    qiLogInfo() << "Attempt #" << i;
    auto doingStuff = middleman.async<bool>("callOnArgument", precious, "doStuff");
    ASSERT_TRUE(doingStuff.value(3000)); // milliseconds
  }
}
