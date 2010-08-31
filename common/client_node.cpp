/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/client_node.hpp>
#include <alcommon-ng/common/detail/client_node_imp.hpp>

namespace AL {
  using namespace Messaging;
  namespace Common {

    ClientNode::ClientNode() {}

    ClientNode::ClientNode(
      const std::string& clientName,
      const std::string& masterAddress) : 
      fImp(boost::shared_ptr<ClientNodeImp>(
        new ClientNodeImp(clientName, masterAddress))) {}

    ClientNode::~ClientNode() {}

    ResultDefinition ClientNode::call(const CallDefinition& callDef) {        
      return fImp->call(callDef);
    }

  }
}
