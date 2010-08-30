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
#include <alcommon-ng/functor/makefunctor.hpp>

namespace AL {
  using namespace Messaging;
  namespace Common {

    void ping() {
      std::cout << "Ping Baby" << std::endl;
    }

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

      // TODO contact the master

      // =========================================================
      // just testing
      addLocalService(ServiceInfo(serverName, serverName, "ping", makeFunctor(&ping)));

      // use the base class client, to send to master
      // todo add serialization of local service.
      //ResultDefinition res = call(CallDefinition("master", "addService"));
      // =========================================================

      boost::thread serverThread( boost::bind(&Server::run, this));
    }

    // shame about this definition of a handler....
    // would be great if we could do R onMessage( {mod, meth, T})
    boost::shared_ptr<AL::Messaging::ResultDefinition> ServerNode::onMessage(const AL::Messaging::CallDefinition &def) {
      // handle message
      std::cout << "  Server: " << fInfo.name << ", received message: " << def.moduleName() << "." << def.methodName() << std::endl;

      std::string key = def.moduleName() + std::string(".") + def.methodName();
      const ServiceInfo& si = getLocalService(key);
      if (si.nodeName == fInfo.name) {
        std::cout << "  Method is for this node " << std::endl;
      } else {

        std::cout << "  Method is for node: " << si.nodeName << std::endl;
      }

      boost::shared_ptr<ResultDefinition> res = boost::shared_ptr<ResultDefinition>(new ResultDefinition());
      si.functor->call(def.args(), res->value());
      return res;
    }

    const NodeInfo& ServerNode::getNodeInfo() const {
      return fInfo;
    }

    void ServerNode::addLocalService(const ServiceInfo& service) {

      std::string key = service.moduleName +
        std::string(".") + service.methodName;

      fLocalServiceList.insert(key, service);
    }

    const ServiceInfo& ServerNode::getLocalService(const std::string& methodHash) {
      // functors ... should be found here
      return fLocalServiceList.get(methodHash);
    }
  }
}
