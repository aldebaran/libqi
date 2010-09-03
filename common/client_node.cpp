/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/client_node.hpp>
#include <string>
#include <alcommon-ng/common/detail/client_node_imp.hpp>

namespace AL {
  using Messaging::CallDefinition;
  using Messaging::ResultDefinition;
  namespace Common {

    ClientNode::ClientNode() {}

    ClientNode::~ClientNode() {}

    ClientNode::ClientNode(
      const std::string& clientName,
      const std::string& masterAddress) :
    fImp(
        new ClientNodeImp(clientName, masterAddress)) {}

    // not in the hxx because not a template
    void ClientNode::callVoid(const std::string& methodName) {
        AL::Messaging::ResultDefinition result;
        void (*f)()  = 0;
        std::string hash = makeSignature(methodName, f);
        xCall(AL::Messaging::CallDefinition(hash), result);
    }

    void ClientNode::xCall(const CallDefinition& callDef, ResultDefinition &result) {
      return fImp->call(callDef, result);
    }

  }
}
