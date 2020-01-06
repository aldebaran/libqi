/*
** Author(s):
**   - Thomas Fontenay <tfontenay@aldebaran.com>
**
** Copyright (c) 2014 Aldebaran Robotics. All rights reserved.
**
*/

#include <vector>
#include <algorithm>
#include <iterator>
#include <future>
#include <thread>
#include <chrono>
#include <thread>
#include <numeric>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qi/session.hpp>
#include <qi/testutils/testutils.hpp>

#include "src/messaging/transportsocketcache.hpp"
#include "src/messaging/tcpmessagesocket.hpp"
#include "src/messaging/transportserver.hpp"

qiLogCategory("TestTransportSocketCache");

using namespace test;
using namespace testing;

namespace {

void newConnection(const std::pair<qi::MessageSocketPtr, qi::Url>& p)
{
  p.first->ensureReading();
}

class TestTransportSocketCache : public ::testing::Test
{
protected:
  TestTransportSocketCache()
  {
    cache_.init();
    server_.newConnection.connect(&newConnection, _1);
  }
  ~TestTransportSocketCache()
  {
    cache_.close();
    server_.close();
  }

  qi::TransportSocketCache cache_;
  qi::TransportServer server_;
};

}


TEST_F(TestTransportSocketCache, DisconnectReconnect)
{
  using namespace qi;
  Promise<void> prom;

  server_.listen("tcp://0.0.0.0:0").wait();
  server_.listen("tcp://0.0.0.0:0").wait();

  auto endpoints = server_.endpoints();
  endpoints.push_back(server_.endpoints().front());
  ServiceInfo servInfo;
  servInfo.setMachineId("tle;l");
  servInfo.setEndpoints(endpoints);
  qiLogDebug() << "CONNECTING: begin";
  qi::Future<qi::MessageSocketPtr> sockfut = cache_.socket(servInfo);
  qi::MessageSocketPtr sock = sockfut.value();
  qiLogDebug() << "CONNECTING: end";
  ASSERT_TRUE(sock->isConnected()) << sock.get();
  qiLogDebug() << "DISCONNECTING: begin";
  sock->disconnect();
  qiLogDebug() << "DISCONNECTING: end";
  ASSERT_FALSE(sock->isConnected()) << sock.get();

  // the disconnected signal can take some time until it's received, so wait a bit
  std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
  qiLogDebug() << "RECONNECTING: begin";
  sockfut = cache_.socket(servInfo);
  sock = sockfut.value();
  qiLogDebug() << "RECONNECTING: END";
  ASSERT_TRUE(sock->isConnected()) << sock.get();
}

TEST_F(TestTransportSocketCache, FirstUrlWillFail)
{
  server_.listen("tcp://127.0.0.1:5555").wait();
  std::vector<qi::Uri> endpoints;
  endpoints.push_back(*qi::uri("tcp://127.0.0.1:4444"));
  endpoints.push_back(server_.endpoints().front());

  qi::ServiceInfo servInfo;
  servInfo.setMachineId(qi::os::getMachineId());
  servInfo.setEndpoints(endpoints);
  qi::Future<qi::MessageSocketPtr> sockFut = cache_.socket(servInfo);
  qi::MessageSocketPtr sock = sockFut.value();

  ASSERT_TRUE(sock->isConnected());
}

static const std::string fakeMachineId = "there is relatively low chances this \
    could end being the same machineID than the actual one of this \
    machine. Then again, one can't be too sure, and we should probably \
    randomly generate it in order to guarantee this test will be consistent.\
    But we can probably take this chance.";

struct TestTransportSocketCacheDifferentMachineIdLocalConnection : TestTransportSocketCache
{
  void SetUp() override
  {
    TestTransportSocketCache::SetUp();

    ASSERT_NE(fakeMachineId, qi::os::getMachineId());

    server_.listen("tcp://127.0.0.1:0").wait();

    // Forge a service info, change the machine ID
    // TransportSocketCache only uses the machineID and the endpoints
    // This will make the cache think the target and ourselves are not
    // on the same machine
    serviceInfo.setMachineId(fakeMachineId);
    serviceInfo.setEndpoints(server_.endpoints());
  }

  qi::ServiceInfo serviceInfo;
};

TEST_F(TestTransportSocketCacheDifferentMachineIdLocalConnection, FailsByDefault)
{
  const auto tentativeSocketFuture = cache_.socket(serviceInfo);

  std::string err;
  ASSERT_TRUE(test::finishesWithError(tentativeSocketFuture, test::willAssignError(err)));
  boost::algorithm::to_lower(err);

  // It says something about reachable endpoint.
  EXPECT_THAT(err, AllOf(HasSubstr("reachable"), HasSubstr("endpoint")));
  // It's not a promise broken.
  EXPECT_THAT(err, Not(AnyOf(HasSubstr("promise"), HasSubstr("broken"))));
}

TEST_F(TestTransportSocketCacheDifferentMachineIdLocalConnection, SuccessAfterInsertion)
{
  const auto endpoint = server_.endpoints().front();
  const auto socket = boost::make_shared<qi::TcpMessageSocket<>>();
  socket->connect(qi::toUrl(endpoint));
  cache_.insert(fakeMachineId, endpoint, socket);

  const auto tentativeSocketFuture = cache_.socket(serviceInfo);
  ASSERT_FALSE(tentativeSocketFuture.hasError());
  ASSERT_EQ(tentativeSocketFuture.value(), socket);

  socket->disconnect();
}



TEST_F(TestTransportSocketCache, SameMachinePublicIp)
{
  qi::MessageSocketPtr client = boost::make_shared<qi::TcpMessageSocket<>>();

  server_.listen("tcp://0.0.0.0:0").wait();
  auto endpoints = server_.endpoints();
  // Find public IP
  const auto uriIt = std::find_if(endpoints.begin(), endpoints.end(),
                                  [](const qi::Uri& uri) {
                                    const auto host = qi::host(*qi::authority(uri));
                                    return !qi::isLoopbackAddress(host);
                                  });
  ASSERT_NE(uriIt, endpoints.end());
  auto uri = *uriIt;
  std::vector<qi::Uri> v;

  v.push_back(uri);
  client->connect(qi::toUrl(uri));
  cache_.insert(qi::os::getMachineId(), uri, client);

  qi::ServiceInfo info;
  info.setMachineId(qi::os::getMachineId());
  info.setEndpoints(v);
  qi::Future<qi::MessageSocketPtr> tentativeSocketFuture = cache_.socket(info);

  tentativeSocketFuture.wait();
  ASSERT_FALSE(tentativeSocketFuture.hasError());
  ASSERT_EQ(tentativeSocketFuture.value(), client);
  client->disconnect();
}

TEST_F(TestTransportSocketCache, ReusesSocketOfServiceWithRelativeEndpoint)
{
  using namespace qi;

  ASSERT_TRUE(test::finishesWithValue(server_.listen("tcp://127.0.0.1:9559")));
  const auto endpoint = server_.endpoints().front();
  const auto machineId = qi::os::getMachineId();

  qi::ServiceInfo info1;
  info1.setMachineId(machineId);
  info1.setName("muffins");
  info1.setEndpoints({ endpoint });

  // This creates a socket that will be connected to `endpoint`.
  const auto socket1Fut = cache_.socket(info1);
  ASSERT_TRUE(test::finishesWithValue(socket1Fut));

  qi::ServiceInfo info2;
  info2.setMachineId(machineId);
  info2.setName("cookies");
  info2.setEndpoints({ *uri("qi:muffins") });

  // Cache should recognize the relative endpoint and return the same socket.
  const auto socket2Fut = cache_.socket(info2);
  ASSERT_TRUE(test::finishesWithValue(socket2Fut));

  EXPECT_EQ(socket1Fut.value(), socket2Fut.value());
}

TEST(TestCall, IPV6Accepted)
{
  // todo: enable whenever qi::Url properly supports ipv6
  return;
  std::string initialEnvValue = qi::os::getenv("QIMESSAGING_ENABLE_IPV6");
  qi::os::setenv("QIMESSAGING_ENABLE_IPV6", "1");

  qi::Url ipv6Url("tcp://[::1]:0");
  ASSERT_TRUE(ipv6Url.isValid());

  qi::TransportServer server;
  qi::Future<void> fut = server.listen(ipv6Url);

  ASSERT_FALSE(fut.hasError());


  auto socket = qi::makeMessageSocket();
  const auto _ = ka::scoped([=]{ socket->disconnect(); });
  fut = socket->connect(ipv6Url);

  ASSERT_FALSE(fut.hasError());

  qi::os::setenv("QIMESSAGING_ENABLE_IPV6", initialEnvValue.c_str());
}

TEST(TestCall, IPV6Rejected)
{
  // todo: enable whenever qi::Url properly supports ipv6
  return;

  std::string initialEnvValue = qi::os::getenv("QIMESSAGING_ENABLE_IPV6");
  qi::os::setenv("QIMESSAGING_ENABLE_IPV6", "");

  qi::Url ipv6Url("tcp://[::1]:4444");
  ASSERT_TRUE(ipv6Url.isValid());

  auto socket = qi::makeMessageSocket();
  const auto _ = ka::scoped([=]{ socket->disconnect(); });
  qi::Future<void> fut = socket->connect(ipv6Url);

  ASSERT_TRUE(fut.hasError());

  qi::TransportServer server;
  fut = server.listen(ipv6Url);

  ASSERT_TRUE(fut.hasError());

  qi::os::setenv("QIMESSAGING_ENABLE_IPV6", initialEnvValue.c_str());
}
