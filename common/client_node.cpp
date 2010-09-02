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

    ClientNode::ClientNode(
      const std::string& clientName,
      const std::string& masterAddress) :
      fImp(boost::shared_ptr<ClientNodeImp>(
        new ClientNodeImp(clientName, masterAddress))) {}

    ClientNode::~ClientNode() {}

    ResultDefinition ClientNode::xCall(const CallDefinition& callDef) {
      return fImp->call(callDef);
    }

    //void ClientNode::call(const std::string& methodName,
    //  AL::Messaging::ReturnValue& result) {
    //  fImp->call(methodName, result);
    //}

    //void ClientNode::call(const std::string& methodName,
    //  const AL::Messaging::ArgumentList& params,
    //  AL::Messaging::ReturnValue& result) {
    //  fImp->call(methodName, params, result);
    //}

    //AL::Messaging::ReturnValue ClientNode::call(const std::string& methodName) {
    //  AL::Messaging::ReturnValue result;
    //  fImp->call(methodName, result);
    //  return result;
    //}

    //AL::Messaging::ReturnValue ClientNode::call(const std::string& methodName,
    //  const AL::Messaging::ArgumentList& params) {
    //  AL::Messaging::ReturnValue result;
    //  fImp->call(methodName, params, result);
    //  return result;
    //}


  }
}
