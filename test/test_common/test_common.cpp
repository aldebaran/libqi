
#include <gtest/gtest.h>
#include <alcommon-ng/common/master_node.hpp>
#include <boost/timer.hpp>
#include <string>


// -- basic types

using namespace AL::Common;
using namespace AL::Messaging;

TEST(NodeTest, test)
{
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

}
