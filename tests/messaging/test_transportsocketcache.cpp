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
    server_.close();
    cache_.close();
  }

  qi::TransportSocketCache cache_;
  qi::TransportServer server_;
};

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

  server_.listen("tcp://127.0.0.1:0");
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
  qiLogInfo() << url.str();
  return host.compare(0,4,"127.") != 0 && host.compare(0, 9, "localhost") != 0;
}

TEST_F(TestTransportSocketCache, SameMachinePublicIp)
{
  qi::TransportSocketPtr client = boost::make_shared<qi::TcpTransportSocket>();

  server_.listen("tcp://0.0.0.0:0");
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

TEST_F(TestTransportSocketCache, LocalIpsUsedOnLocalConnections)
{
  // generate plenty of endpoints
  qi::Future<void> listenFut = server_.listen("tcp://0.0.0.0:0");
  listenFut = server_.listen("tcp://0.0.0.0:0");
  listenFut = server_.listen("tcp://0.0.0.0:0");

  qi::UrlVector serverEndpoints = server_.endpoints();
  qi::ServiceInfo info;
  info.setMachineId(qi::os::getMachineId());
  info.setEndpoints(serverEndpoints);

  qi::Future<qi::TransportSocketPtr> tentativeSocketFuture = cache_.socket(info, "");
  tentativeSocketFuture.wait();
  if (tentativeSocketFuture.hasError())
    qiLogError() << tentativeSocketFuture.error();
  ASSERT_FALSE(tentativeSocketFuture.hasError());
  ASSERT_TRUE(tentativeSocketFuture.value()->isConnected());
  ASSERT_FALSE(publicIp(tentativeSocketFuture.value()->remoteEndpoint()));
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
