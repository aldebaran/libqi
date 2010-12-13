
#include <gtest/gtest.h>
#include <qi/messaging.hpp>
#include <qi/perf/dataperftimer.hpp>
#include <qi/perf/sleep.hpp>
#include <string>
#include <alvalue.pb.h>

using namespace qi;

std::string gMasterAddress = "127.0.0.1:5555";
std::string gServerName = "server";
std::string gServerAddress = "127.0.0.1:5556";

//Context* c = new Context();
//Master gMaster(gMasterAddress);
//Server gServer("server");
//Client gClient("client");

std::string echo(const std::string& in) {
  return in;
}
//
//// SERVER
//TEST(ServerTest, createWithNoArgs)
//{
//  Server server;
//}
//
//TEST(ServerTest, createWithJustName)
//{
//  Server server("server");
//}
//
//TEST(ServerTest, createWithNameAndContext)
//{
//  //sleep(10);
//  Context * c = new Context();
//  Server server("server", c);
//  delete(c);
//}
//
//// CLIENT
//TEST(ClientTest, createWithNoArgs)
//{
//  Client client;
//}
//
//TEST(ClientTest, createWithJustName)
//{
//  Client client("client");
//}
//
//TEST(ClientTest, createWithNameAndContext)
//{
//  //sleep(10);
//  Context * c = new Context();
//  Client client("client", c);
//  delete(c);
//}
//
//// PUBLISHER
//TEST(PublisherTest, createWithNoArgs)
//{
//  Publisher publisher;
//}
//
//TEST(PublisherTest, createWithJustName)
//{
//  Publisher publisher("publisher");
//}
//
//TEST(PublisherTest, createWithNameAndContext)
//{
//  //sleep(10);
//  Context * c = new Context();
//  Publisher publisher("publisher", c);
//  delete(c);
//}
//
//// SUBSCRIBER
//TEST(SubscriberTest, createWithNoArgs)
//{
//  Subscriber subscriber;
//}
//
//TEST(SubscriberTest, createWithJustName)
//{
//  Subscriber subscriber("subscriber");
//}
//
//TEST(SubscriberTest, createWithNameAndContext)
//{
//  //sleep(10);
//  Context * c = new Context();
//  Subscriber subscriber("subscriber", c);
//  delete(c);
//}
//
//
//// MASTER
//TEST(MasterTest, createWithNoArgs)
//{
//  Master master;
//}
//
//TEST(MasterTest, createWithJustPort)
//{
//  // TODO int argument
//  Master master("5555");
//}
//
//TEST(MasterTest, createWithJustIPDEPRECATE)
//{
//  // TODO deprecate this
//  Master master("127.0.0.1");
//}
//
//TEST(MasterTest, createWithStupidPortDEPRECATE)
//{
//
//  Master master("127.0.0.1:dfjkl");
//}
//
//TEST(MasterTest, createWithContext)
//{
//  //sleep(10);
//  // TODO deprecate this
//  Context* c = new Context();
//  Master master("127.0.0.1:5555", c);
//  delete(c);
//}



//
//TEST(ClientServer, explicitAddressPlusContext)
//{
//  //sleep(10);
//  Context* c = new Context();
//  std::string address = "127.0.0.1:5555";
//  Master master(address, c);
//  master.run();
//  Server server("server", c);
//  server.connect(address);
//  server.advertiseService("server.echo", &echo);
//  Client client("client", c);
//  client.connect(address);
//  std::string arg = "hello world";
//  std::string ret = client.call<std::string>("server.echo", arg);
//  ASSERT_EQ(arg, ret);
//  delete(c);
//}
//
//TEST(ClientServer, explicitAddressPlusContext2)
//{
//  //sleep(10);
//  Context* c = new Context();
//  std::string address = "127.0.0.1:5555";
//  Master master(address, c);
//  master.run();
//  Server server("server", c);
//  server.connect(address);
//  server.advertiseService("server.echo", &echo);
//  Client client("client", c);
//  client.connect(address);
//  std::string arg = "hello world";
//  std::string ret = client.call<std::string>("server.echo", arg);
//  ASSERT_EQ(arg, ret);
//  delete(c);
//}


TEST(ClientServer, defaultArgs)
{
  Master master;
  master.run();
  Server server;
  server.connect();
  server.advertiseService("server.echo", &echo);
  Client client;
  client.connect();
  std::string arg = "hello world";
  std::string ret = client.call<std::string>("server.echo", arg);
  ASSERT_EQ(arg, ret);
}
//
//TEST(ClientServer, explicitAddress)
//{
//  sleep(10);
//  std::string address = "127.0.0.1:5555";
//  Master master(address);
//  master.run();
//  Server server;
//  server.connect(address);
//  server.advertiseService("server.echo", &echo);
//  Client client;
//  client.connect(address);
//  std::string arg = "hello world";
//  std::string ret = client.call<std::string>("server.echo", arg);
//  ASSERT_EQ(arg, ret);
//}
//
//TEST(ClientServer, explicitAddressPlusOne)
//{
//  std::string address = "127.0.0.1:5556";
//  Master master(address);
//  master.run();
//  Server server;
//  server.connect(address);
//  server.advertiseService("server.echo", &echo);
//  Client client;
//  client.connect(address);
//  std::string arg = "hello world";
//  std::string ret = client.call<std::string>("server.echo", arg);
//  ASSERT_EQ(arg, ret);
//}

//
//void ping() {
//}
//
//ALCompat::ALValue echo_proto(const ALCompat::ALValue& in) {
//  ALCompat::ALValue val = in;
//  return val;
//}
//
//
//std::string echo(const std::string& in) {
//  return in;
//}
//
//TEST(Client, createWithStupidMasterAddress)
//{
//  Client client("client", c);
//  client.connect("blabla");
//  bool ex = false;
//  try {
//    client.callVoid("ognagnuk");
//  }
//  catch( const std::exception&) {
//    ex = true;
//  }
//  EXPECT_EQ(true, ex);
//}
//
//TEST(Server, createWithStupidMasterAddress)
//{
//  Server server("server", c);
//  server.connect("oink");
//  bool ex = false;
//  try {
//    server.advertiseService("ognagnuk", &ping);
//  }
//  catch( const std::exception&) {
//    ex = true;
//  }
//  EXPECT_EQ(true, ex);
//}
//
//TEST(Master, createWithStupidMasterAddress)
//{
//  Master master("oink2", c);
//  // should not blow up, but we should be able to find if it is happy
//}
//

