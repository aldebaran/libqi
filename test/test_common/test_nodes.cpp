
#include <gtest/gtest.h>
#include <alcommon-ng/common/common.hpp>
#include <boost/timer.hpp>
#include <string>

using namespace AL::Common;
using namespace AL::Messaging;

std::string gMasterAddress = "127.0.0.1:5555";
std::string gServerName = "server";
std::string gServerAddress = "127.0.0.1:5556";

TEST(ClientNode, createWithStupidMasterPort) 
{
  ClientNode client("client", "blabla");
  client.call("ognagnuk");
}

TEST(ServerNode, createWithStupidMasterPort) 
{
  ServerNode server("server", "blabla", "oink");
}

TEST(MasterNode, createWithStupidMasterPort) 
{
  MasterNode master("oink2");
}

TEST(ServerNode, createWithStupidServerPort) 
{
  MasterNode master("127.0.0.1:6666");
  ServerNode server("server", "blabla", "127.0.0.1:6666");
}



MasterNode gMaster(gMasterAddress);
ServerNode gServer(gServerName, gServerAddress, gMasterAddress);
ClientNode gClient("client", gMasterAddress);

TEST(Nodes, NormalUsage)
{
  std::cout << "TEST: Initialized " << std::endl;
  std::cout << "TEST: Calling master.listServices " << std::endl;
  ReturnValue result1 = gClient.call("master.listServices");

  std::cout << "TEST: Calling server.ping " << std::endl;
  ReturnValue result2 = gClient.call("server.ping");

  std::cout << "TEST: Calling master.gobledigook " << std::endl;
  ReturnValue result3 = gClient.call("master.gobledigook");

  for(int i=0; i<100; i++) {
    ReturnValue result4 = gClient.call("master.listServices");
  }
}
