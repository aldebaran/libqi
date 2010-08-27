/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/master_node.hpp>

namespace AL {
  namespace Common {

    // TODO create a client only node as a base class
    MasterNode::MasterNode(
      const std::string& masterName,
      const std::string& masterAddress) :
        ServerNode(masterName, masterAddress, masterAddress) {

      // just testing
      //addService(ServiceInfo(name,name,"addNode"));
    }

    //void Node::addNode(const NodeInfo& node) {
    //  // TODO verify validity
    //  fNodeList.insert(
    //    std::make_pair<std::string, NodeInfo>(node.name, node));
    //}

    //const NodeInfo& Node::getNode(const std::string& name) const {
    //  NameLookup<NodeInfo>::const_iterator it = fNodeList.find(name);
    //  if (it != fNodeList.end()) {
    //    return it->second;
    //  }
    //  return fInvalidNode;
    //}

    //void Node::addService(const ServiceInfo& service) {
    //  std::string key = service.moduleName +
    //    std::string(".") +
    //    service.methodName;

    //  fServiceList.insert(
    //    std::make_pair<std::string, ServiceInfo>(key, service));
    //}

    //const ServiceInfo& Node::getService(const std::string& name) const {
    //  NameLookup<ServiceInfo>::const_iterator it = fServiceList.find(name);
    //  if (it != fServiceList.end()) {
    //    return it->second;
    //  }
    //  return fInvalidService;
    //}

  }
}
