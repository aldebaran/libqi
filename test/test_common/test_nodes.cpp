
#include <gtest/gtest.h>
#include <alcommon-ng/common/common.hpp>
#include <alcommon-ng/tools/dataperftimer.hpp>
#include <boost/timer.hpp>
#include <string>
#include <cmath>

using namespace AL::Common;
using namespace AL::Messaging;

std::string gMasterAddress = "127.0.0.1:5555";
std::string gServerName = "server";
std::string gServerAddress = "127.0.0.1:5556";


MasterNode gMaster(gMasterAddress);
ServerNode gServer(gServerName, gServerAddress, gMasterAddress);
ClientNode gClient("client", gMasterAddress);

void ping() {
}

std::string echo(const std::string& in) {
  return in;
}


TEST(Nodes, PerformancePing)
{

  gServer.addService("wibble.ping", &ping);
  unsigned int numMessages = 10000;

  AL::Test::DataPerfTimer dt("Node void -> ping -> void");
  dt.start(numMessages);
  for (unsigned int loop = 0; loop < numMessages; loop++) {
    // Serialize
    gClient.callVoid("wibble.ping");
  }
  dt.stop();
}

TEST(Nodes, PerformanceEcho)
{
  gServer.addService("wibble.echo", &echo);
  unsigned int numMessages = 10000;
  unsigned int numPowers = 12;
  AL::Test::DataPerfTimer dt("Node string -> echo -> string");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string request = std::string(numBytes, character);

    dt.start(numMessages, numBytes);
    for (unsigned int loop = 0; loop < numMessages; loop++) {
      // Serialize
      std::string result = gClient.call<std::string>("wibble.echo", request);
    }
    dt.stop();
  }
}



TEST(ClientNode, createWithStupidMasterPort)
{
  ClientNode client("client", "blabla");
  bool ex = false;
  try {
    client.callVoid("ognagnuk");
  }
  catch( const std::exception& e) {
    ex = true;
  }
  EXPECT_EQ(true, ex);
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



//TEST(Nodes, NormalUsage)
//{
//  //std::cout << "TEST: Initialized " << std::endl;
//  //std::cout << "TEST: Calling master.listServices " << std::endl;
//  //ReturnValue result1 = gClient.call("master.listServices");
//
//  std::cout << "TEST: Binding wibble.echo " << std::endl;
//  gServer.addService("wibble.echo", &echo);
//  std::string s = gClient.call<std::string>("wibble.echo", std::string("errr"));
//  //std::cout << "TEST: Calling wibble.echo " << std::endl;
//
//  std::cout << "TEST: Binding wibble.ping " << std::endl;
//  gServer.addService("wibble.ping", &ping);
//  gClient.callVoid("wibble.ping");
//  //std::cout << "TEST: Calling wibble.ping " << std::endl;
//
//  std::cout << "TEST: Calling master.gobledigook " << std::endl;
//  gClient.callVoid("master.gobledigook");
//}
//
