/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/nodes/client_node.hpp>
#include <string>
#include <qi/nodes/detail/client_node_imp.hpp>
#include <qi/serialization/serializer.hpp>

namespace qi {

  /// <summary>
  /// Used to call services that have been added to a server. If the service
  /// is unknown, the master is interogated to find the appropriate server.
  /// </summary>
  ClientNode::ClientNode() {}

  /// <summary> Destructor. </summary>
  ClientNode::~ClientNode() {}

  /// <summary>
  /// DefaultConstructor Used to call services that have been added to a
  /// server. If the service is unknown, the master is interogated
  /// to find the appropriate server.
  /// </summary>
  /// <param name="clientName"> Name of the client. </param>
  /// <param name="masterAddress"> The master address. </param>
  ClientNode::ClientNode(const std::string& clientName,
                         const std::string& masterAddress)
    : fImp(new detail::ClientNodeImp(clientName, masterAddress))
  {}

  void ClientNode::callVoid(const std::string& methodName) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    void (*f)()  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.write<std::string>(hash);
    xCall(hash, calldef, resultdef);
  }

  void ClientNode::xCall(const std::string &signature,
    const qi::serialization::BinarySerializer& callDef,
    qi::serialization::BinarySerializer &result) {
    return fImp->call(signature, callDef, result);
  }
}
