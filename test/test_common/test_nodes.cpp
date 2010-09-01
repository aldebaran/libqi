
#include <gtest/gtest.h>
#include <alcommon-ng/common/common.hpp>
#include <alcommon-ng/tools/dataperftimer.hpp>
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


std::string echo(const std::string& in) {
  return in;
}



TEST(Nodes, NormalUsage)
{
  std::cout << "TEST: Initialized " << std::endl;
  std::cout << "TEST: Calling master.listServices " << std::endl;
  ReturnValue result1 = gClient.call("master.listServices");

  std::cout << "TEST: Binding wibble.echo " << std::endl;
  gServer.addService("wibble.echo", &echo);
  std::cout << "TEST: Calling wibble.echo " << std::endl;

  std::cout << "TEST: Calling master.gobledigook " << std::endl;
  ReturnValue result4 = gClient.call("master.gobledigook");

  //for(int i=0; i<100; i++) {
  //  ReturnValue result5 = gClient.call("master.listServices");
  //}
}

TEST(Nodes, Performance)
{
  unsigned int numMessages = 1000;
  unsigned int numPowers = 12;
  AL::Test::DataPerfTimer dt("NodeCalls");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);
    ArgumentList args;
    ReturnValue result;
    args.push_back(request);

    dt.start(numMessages, numBytes);
    for (unsigned int loop = 0; loop < numMessages; loop++) {
      // Serialize
      gClient.call("wibble.echo", args, result);
    }
    dt.stop();
  }
  
}
