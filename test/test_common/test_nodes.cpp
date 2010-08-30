
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

TEST(Nodes, NormalUsage)
{
  MasterNode master(gMasterName, gMasterAddress);
  ServerNode server(gServerName, gServerAddress, gMasterAddress);
  ClientNode client("client", gMasterAddress);

  client.call(CallDefinition("master", "listServices"));
}
