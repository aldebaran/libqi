/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/server_node.hpp>
#include <alcommon-ng/messaging/messaging.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace AL {
  using namespace Messaging;
  namespace Common {

    // dodgy?
    static ServiceInfo fInvalidService;

    // TODO create a client only node as a base class
    ServerNode::ServerNode(
      const std::string& serverName,
      const std::string& serverAddress,
      const std::string& masterAddress) :
        AL::Common::ClientNode(serverName, masterAddress),
        AL::Messaging::DefaultServer(serverAddress)
     {
      fInfo.name = serverName;
      fInfo.address = serverAddress;

      setMessageHandler(this);

      // just testing
      //addService(ServiceInfo(serverName, serverName, "listServices"));

      boost::thread serverThread( boost::bind(&Server::run, this));
    }

    // shame about this definition of a handler....
    // would be great if we could do R onMessage( {mod, meth, T})
    boost::shared_ptr<AL::Messaging::ResultDefinition> ServerNode::onMessage(const AL::Messaging::CallDefinition &def) {
      // handle message
      std::cout << fInfo.name << " received message: " << def.moduleName() << "." << def.methodName() << std::endl;

      std::string key = def.moduleName() + std::string(".") + def.methodName();
      const ServiceInfo& si = getService(key);
      if (si.nodeName == fInfo.name) {
        std::cout << " Method is for this node " << std::endl;
      } else {

        std::cout << " Method is for node: " << si.nodeName << std::endl;

        //const NodeInfo& ni = getNode(si.nodeName);
        //if (ni.address != "") {
        //  std::cout << " Node is at address: " << ni.address << std::endl;
        //}
      }
      // TODO find the local or remote functor
     // boost::bind<NodeInfo>(&Node::getNode, _1);

      boost::shared_ptr<ResultDefinition> res =
        boost::shared_ptr<ResultDefinition>(new ResultDefinition());
      return res;
    }

    const NodeInfo& ServerNode::getNodeInfo() const {
      return fInfo;
    }

    void ServerNode::addService(const ServiceInfo& service) {
      std::string key = service.moduleName +
        std::string(".") + service.methodName;

      fLocalServiceList.insert(
        std::make_pair<std::string, ServiceInfo>(key, service));
    }

    const ServiceInfo& ServerNode::getService(const std::string& methodHash) const {
      // functors ... should be found here
      NameLookup<ServiceInfo>::const_iterator it = fLocalServiceList.find(methodHash);
      if (it != fLocalServiceList.end()) {
        return it->second;
      }
      return fInvalidService;
    }
  }
}
