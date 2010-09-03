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

    /// <summary> Default constructor. </summary>
    ServerNode::ServerNode() {}
    ServerNode::~ServerNode() {}

    /// <summary> Default constructor. </summary>
    /// <param name="serverName"> Name of the server. </param>
    /// <param name="serverAddress"> The server address. </param>
    /// <param name="masterAddress"> The master address. </param>
    ServerNode::ServerNode(
      const std::string& serverName,
      const std::string& serverAddress,
      const std::string& masterAddress) :
    fImp(
      new ServerNodeImp(serverName, serverAddress, masterAddress))
      {}

    void ServerNode::xAddService(const std::string& name, Functor* functor) {
      fImp->addService(name, functor);
    }
  }
}
