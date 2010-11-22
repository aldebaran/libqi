
#include <gtest/gtest.h>
#include <string>
#include <boost/timer.hpp>
#include <qi/messaging.hpp>
#include <qi/perf/sleep.hpp>

using namespace qi;

std::string gMasterAddress = "tcp://127.0.0.1:5555";
std::string gServerName = "server";

TEST(MasterServerClient, creation)
{
  Master master(gMasterAddress);
  Server server(gServerName, gMasterAddress);
  Client client("client", gMasterAddress);
}

TEST(ClientTest, creation)
{
  Client client("client", gMasterAddress);
}

TEST(ClientTest, multipleCreation)
{
  for (int i = 0; i< 100; i++) {
    Client client("client", gMasterAddress);
  }
}



//TEST(ServerTest, creation)
//{
//  Server server(gServerName, gServerAddress, gMasterAddress);
//  Sleep(1000);
//}

//TEST(ServerTest, multipleCreationNewPorts)
//{
//  // PROBLEM: limited by number of threads !?
//
//  int numServers = 5; // at 12 it crashes
//  for (int i = 5560; i< 5560 + numServers; i++) {
//
//    std::stringstream ss;
//    ss << "tcp://127.0.0.1:";
//    ss << i;
//    std::string serverAddress = ss.str();
//    std::cout << "Creating Server " << serverAddress << std::endl;
//    Server server(gServerName, gMasterAddress);
//    // after about 12 services ... crash
//    // not enough storage space is available ../src/thread.cpp
//    // sometimes a pthread create error.
//    sleep(1);
//    std::cout << "Created Server " << serverAddress << std::endl;
//  }
//}

TEST(ServerTest, multipleCreationSamePort)
{
  // PROBLEM: starting servers too fast crashes
  // PROBLEM: server object should throw, or needs OK? method
  for (int i = 0; i< 10; i++) {
    std::cout << "Creating Server " << i << std::endl;
    Server server(gServerName, gMasterAddress);
    // without this sleep, bad things happen!!!
    sleep(1);
    std::cout << "Created Server " << i << std::endl;
  }
}




int test(const int &t)
{
  return t + 42;
}

TEST(MasterTest, creation)
{
  Master master(gMasterAddress);
  sleep(1);
}

TEST(MasterTest, nodeInfo)
{
  Master master(gMasterAddress);
  //sleep(1);
  //NodeInfo ni = master.getNodeInfo();
  //EXPECT_EQ(gMasterName, ni.name);
  //EXPECT_EQ(gMasterAddress, ni.address);
}

TEST(MasterTest, serviceInfo)
{
  Master master(gMasterAddress);
  //sleep(1);
  //ServiceInfo si("n", "mod", "meth", qi::makeFunctor(&test));
  //master.addLocalService(si);
  //ServiceInfo res = master.getLocalService("mod.meth");
  //EXPECT_EQ(si.nodeName, res.nodeName);
  //EXPECT_EQ(si.moduleName, res.moduleName);
  //EXPECT_EQ(si.methodName, res.methodName);
}



  //std::string gAddress = "tcp://127.0.0.1:5555";
  //Master master("master", gAddress);

  //ServiceInfo s = master.getService("master.addNode");
  //EXPECT_EQ("master", s.nodeName);
  //EXPECT_EQ("master", s.moduleName);
  //EXPECT_EQ("addNode", s.methodName);

  //Client client(gAddress);
  //CallDefinition def;
  //def.methodName() = "addNode";
  //def.moduleName() = "master";
  //ResultDefinition ret = client.send(def);

