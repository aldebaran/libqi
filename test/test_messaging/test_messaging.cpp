
#include <gtest/gtest.h>
#include <qi/messaging.hpp>
#include <qi/perf/dataperftimer.hpp>
#include <string>
#include <alvalue.pb.h>

using namespace qi;

std::string gMasterAddress = "127.0.0.1:5555";
std::string gServerName = "server";
std::string gServerAddress = "127.0.0.1:5556";

Master gMaster;
Server gServer(gServerName);
Client gClient("client");



void ping() {
}

ALCompat::ALValue echo_proto(const ALCompat::ALValue& in) {
  ALCompat::ALValue val = in;
  return val;
}


std::string echo(const std::string& in) {
  return in;
}

TEST(Client, createWithStupidMasterAddress)
{
  Client client("client");
  client.connect("blabla");
  bool ex = false;
  try {
    client.callVoid("ognagnuk");
  }
  catch( const std::exception&) {
    ex = true;
  }
  EXPECT_EQ(true, ex);
}

TEST(Server, createWithStupidMasterAddress)
{
  Server server("server");
  server.connect("oink");
  bool ex = false;
  try {
    server.advertiseService("ognagnuk", &ping);
  }
  catch( const std::exception&) {
    ex = true;
  }
  EXPECT_EQ(true, ex);
}

TEST(Master, createWithStupidMasterAddress)
{
  Master master("oink2");
  // should not blow up, but we should be able to find if it is happy
}

TEST(Messaging, PerformancePing)
{

  gServer.advertiseService("wibble.ping", &ping);
  unsigned int numMessages = 10000;

  qi::perf::DataPerfTimer dt("Node void -> ping -> void");
  dt.start(numMessages);
  for (unsigned int loop = 0; loop < numMessages; loop++) {
    // Serialize
    gClient.callVoid("wibble.ping");
  }
  dt.stop();
}

TEST(Messaging, PerformanceEcho)
{
  gServer.advertiseService("wibble.echo", &echo);
  unsigned int numMessages = 10000;
  unsigned int numPowers = 12;
  qi::perf::DataPerfTimer dt("Node string -> echo -> string");
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

TEST(Messaging, PerformanceEchoProtobuf)
{
  gServer.advertiseService("wibble.echo", &echo_proto);
  unsigned int numMessages = 10000;
  unsigned int numPowers = 12;
  qi::perf::DataPerfTimer dt("Node string -> echo -> string");
  char character = 'A';

  // loop message sizes 2^i bytes
  for (unsigned int i = 1; i < numPowers; i++) {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string requeststr = std::string(numBytes, character);

    ALCompat::ALValue request;
    request.set_type(ALCompat::ALValue::STRING);
    request.set_stringvalue(requeststr);
    dt.start(numMessages, numBytes);
    for (unsigned int loop = 0; loop < numMessages; loop++) {
      // Serialize
      ALCompat::ALValue result = gClient.call<ALCompat::ALValue>("wibble.echo", request);
    }
    dt.stop();
  }
}





//TEST(Messaging, NormalUsage)
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
