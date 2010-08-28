
#include <gtest/gtest.h>
#include <alcommon-ng/common/common.hpp>
#include <boost/timer.hpp>
#include <string>

using namespace AL::Common;
using namespace AL::Messaging;

std::string gMasterName = "master"; // avoid this?
std::string gMasterAddress = "tcp://127.0.0.1:5555";
std::string gServerName = "server";
std::string gServerAddress = "tcp://127.0.0.1:5556";

TEST(MasterServerClient, creation)
{
  MasterNode master(gMasterName, gMasterAddress);
  ServerNode server(gServerName, gServerAddress, gMasterAddress);
  ClientNode client("client", gMasterAddress);
}

TEST(ClientNodeTest, creation)
{
  ClientNode client("client", gMasterAddress);
}

TEST(ClientNodeTest, multipleCreation)
{
  for (int i = 0; i< 100; i++) {
    ClientNode client("client", gMasterAddress);
  }
}



//TEST(ServerNodeTest, creation)
//{
//  ServerNode server(gServerName, gServerAddress, gMasterAddress);
//  Sleep(1000);
//}

TEST(ServerNodeTest, multipleCreationNewPorts)
{
  // PROBLEM: limited by number of threads !?

  int numServers = 5; // at 12 it crashes
  for (int i = 5560; i< 5560 + numServers; i++) {
    
    std::stringstream ss;
    ss << "tcp://127.0.0.1:";
    ss << i;
    std::string serverAddress = ss.str();
    std::cout << "Creating Server " << serverAddress << std::endl;
    ServerNode server(gServerName, serverAddress, gMasterAddress);
    // after about 12 services ... crash
    // not enough storage space is available ../src/thread.cpp
    // sometime a pthread create error.
    Sleep(1000);
    std::cout << "Created Server " << serverAddress << std::endl;
  }
}

TEST(ServerNodeTest, multipleCreationSamePort)
{
  // PROBLEM: starting servers too fast crashes
  // PROBLEM: server object should throw, or needs OK? method
  for (int i = 0; i< 10; i++) {
    std::cout << "Creating Server " << i << std::endl;
    ServerNode server(gServerName, gServerAddress, gMasterAddress);
    // without this sleep, bad things happen!!!
    Sleep(1000);
    std::cout << "Created Server " << i << std::endl;
  }
}




TEST(ServerNodeTest, nodeInfo)
{
  ServerNode server(gServerName, gServerAddress, gMasterAddress);
  Sleep(1000);
  NodeInfo ni = server.getNodeInfo(); 
  EXPECT_EQ(gServerName, ni.name);
  EXPECT_EQ(gServerAddress, ni.address);
}

TEST(ServerNodeTest, serviceInfo)
{
  ServerNode server(gServerName, gServerAddress, gMasterAddress);
  Sleep(1000);
  ServiceInfo si("n", "mod", "meth");
  server.addService(si);
  ServiceInfo res = server.getService("mod.meth"); 
  EXPECT_EQ(si.nodeName, res.nodeName);
  EXPECT_EQ(si.moduleName, res.moduleName);
  EXPECT_EQ(si.methodName, res.methodName);
}

TEST(MasterNodeTest, creation)
{
  MasterNode master(gMasterName, gMasterAddress);
  Sleep(1000);
}

TEST(MasterNodeTest, nodeInfo)
{
  MasterNode master(gMasterName, gMasterAddress);
  Sleep(1000);
  NodeInfo ni = master.getNodeInfo(); 
  EXPECT_EQ(gMasterName, ni.name);
  EXPECT_EQ(gMasterAddress, ni.address);
}

TEST(MasterNodeTest, serviceInfo)
{
  MasterNode master(gMasterName, gMasterAddress);
  Sleep(1000);
  ServiceInfo si("n", "mod", "meth");
  master.addService(si);
  ServiceInfo res = master.getService("mod.meth"); 
  EXPECT_EQ(si.nodeName, res.nodeName);
  EXPECT_EQ(si.moduleName, res.moduleName);
  EXPECT_EQ(si.methodName, res.methodName);
}



  //std::string gAddress = "tcp://127.0.0.1:5555";
  //MasterNode master("master", gAddress);

  //ServiceInfo s = master.getService("master.addNode");
  //EXPECT_EQ("master", s.nodeName);
  //EXPECT_EQ("master", s.moduleName);
  //EXPECT_EQ("addNode", s.methodName);

  //ClientNode client(gAddress);
  //CallDefinition def;
  //def.methodName() = "addNode";
  //def.moduleName() = "master";
  //ResultDefinition ret = client.send(def);

