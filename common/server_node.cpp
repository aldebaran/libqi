/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/server_node.hpp>
#include <string>
#include <alcommon-ng/common/detail/server_node_imp.hpp>

namespace AL {
  namespace Common {

    ServerNode::ServerNode() {}

    ServerNode::ServerNode(
      const std::string& serverName,
      const std::string& serverAddress,
      const std::string& masterAddress) :
    fImp(boost::shared_ptr<ServerNodeImp>(
      new ServerNodeImp(serverName, serverAddress, masterAddress)))
      {}


    void ServerNode::addService(const ServiceInfo& service) {
      fImp->addService(service);
    }
  }
}
