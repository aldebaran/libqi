
#include "gtest/gtest.h"
#include <alcommon-ng/common/node.h>
#include <alcommon-ng/messaging/client.hpp>
#include <boost/timer.hpp>
#include <string>


// -- basic types

using namespace AL::Common;
using namespace AL::Messaging;

TEST(NodeTest, test)
{
  std::string gAddress = "tcp://127.0.0.1:5555";
  Node n("master", gAddress);
  ServiceInfo s = n.getService("master.addNode");
  EXPECT_EQ("master", s.nodeName);
  EXPECT_EQ("master", s.moduleName);
  EXPECT_EQ("addNode", s.methodName);

  Client client(gAddress);
  CallDefinition def;
  def.setMethodName("addNode");
  def.setModuleName("master");
  def.setRequestId(1);
  boost::shared_ptr<ResultDefinition> ret = client.send(def);

}
