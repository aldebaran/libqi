/*
** Author(s):
**   - Thomas Fontenay <tfontenay@aldebaran.com>
**
** Copyright (c) 2014 Aldebaran Robotics. All rights reserved.
**
*/

#include <algorithm>

#include <gtest/gtest.h>

#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qi/session.hpp>

#include "src/messaging/transportsocketcache.hpp"
#include "src/messaging/tcptransportsocket.hpp"
#include "src/messaging/transportserver.hpp"

qiLogCategory("TestTransportSocketCache");

namespace {

void newConnection(qi::TransportSocketPtr socket)
{
  socket->startReading();
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
  qi::Promise<void> prom;

  server_.listen("tcp://0.0.0.0:0").wait();
  server_.listen("tcp://0.0.0.0:0").wait();

  std::vector<qi::Url> endpoints = server_.endpoints();
  //endpoints.push_back(server_.endpoints()[0]);
  qi::ServiceInfo servInfo;
  servInfo.setMachineId("tle;l");
  servInfo.setEndpoints(endpoints);
  qi::Future<qi::TransportSocketPtr> sockfut = cache_.socket(servInfo, endpoints[0].protocol());
  qi::TransportSocketPtr sock = sockfut.value();
  ASSERT_TRUE(sock->isConnected());
  sock->disconnect();
  ASSERT_FALSE(sock->isConnected());
  sockfut = cache_.socket(servInfo, endpoints[0].protocol());
  sock = sockfut.value();
  ASSERT_TRUE(sock->isConnected());
}

TEST_F(TestTransportSocketCache, FirstUrlWillFail)
{
  server_.listen("tcp://127.0.0.1:5555").wait();
  qi::UrlVector endpoints;
  endpoints.push_back("tcp://127.0.0.1:4444");
  endpoints.push_back(server_.endpoints()[0]);

  qi::ServiceInfo servInfo;
  servInfo.setMachineId(qi::os::getMachineId());
  servInfo.setEndpoints(endpoints);
  qi::Future<qi::TransportSocketPtr> sockFut = cache_.socket(servInfo, endpoints[0].protocol());
  qi::TransportSocketPtr sock = sockFut.value();

  ASSERT_TRUE(sock->isConnected());
}

TEST_F(TestTransportSocketCache, DifferentMachineIdLocalConnection)
{
  qi::TransportSocketPtr socket = boost::make_shared<qi::TcpTransportSocket>();
  const std::string& fakeMachineId = "there is relatively low chances this \
      could end being the same machineID than the actual one of this \
      machine. Then again, one can't be too sure, and we should probably \
      randomly generate it in order to guarantee this test will be consistent.\
      Butwe can probably take this chance.";

  ASSERT_NE(fakeMachineId, qi::os::getMachineId());

  server_.listen("tcp://127.0.0.1:0").wait();
  qi::Url endpoint = server_.endpoints()[0];
  socket->connect(endpoint);
  cache_.insert(fakeMachineId, endpoint, socket);


  // Forge a service info, change the machine ID
  // TransportSocketCache only uses the machineID and the endpoints
  qi::ServiceInfo serviceInfo;
  // This will make the cache think the target and ourselves are not
  // on the same machine
  serviceInfo.setMachineId(fakeMachineId);
  serviceInfo.setEndpoints(server_.endpoints());

  qi::Future<qi::TransportSocketPtr> tentativeSocketFuture = cache_.socket(serviceInfo, "");

  ASSERT_FALSE(tentativeSocketFuture.hasError());
  ASSERT_EQ(tentativeSocketFuture.value(), socket);
  socket->disconnect();
}

static bool publicIp(const qi::Url& url)
{
  const std::string& host = url.host();
  bool ispublic= host.find("127.") == std::string::npos && host.find("localhost") == std::string::npos;
  return ispublic;
}

TEST_F(TestTransportSocketCache, SameMachinePublicIp)
{
  qi::TransportSocketPtr client = boost::make_shared<qi::TcpTransportSocket>();

  server_.listen("tcp://0.0.0.0:0").wait();
  qi::UrlVector endpoints = server_.endpoints();
  // Find public IP
  qi::UrlVector::iterator urlIt = std::find_if(endpoints.begin(), endpoints.end(), &publicIp);
  ASSERT_NE(urlIt, endpoints.end());
  qi::Url url = *urlIt;
  qi::UrlVector v;

  v.push_back(url);
  client->connect(url);
  cache_.insert(qi::os::getMachineId(), url, client);

  qi::ServiceInfo info;
  info.setMachineId(qi::os::getMachineId());
  info.setEndpoints(v);
  qi::Future<qi::TransportSocketPtr> tentativeSocketFuture = cache_.socket(info, "");

  tentativeSocketFuture.wait();
  ASSERT_FALSE(tentativeSocketFuture.hasError());
  ASSERT_EQ(tentativeSocketFuture.value(), client);
  client->disconnect();
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


  qi::TransportSocketPtr socket = qi::makeTransportSocket("tcp");
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

  qi::TransportSocketPtr socket = qi::makeTransportSocket("tcp");
  qi::Future<void> fut = socket->connect(ipv6Url);

  ASSERT_TRUE(fut.hasError());

  qi::TransportServer server;
  fut = server.listen(ipv6Url);

  ASSERT_TRUE(fut.hasError());

  qi::os::setenv("QIMESSAGING_ENABLE_IPV6", initialEnvValue.c_str());
}



int main(int ac, char **av)
{
  qi::Application app(ac, av);
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
