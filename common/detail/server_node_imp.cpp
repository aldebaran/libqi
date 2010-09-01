/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/detail/server_node_imp.hpp>
#include <string>
#include <alcommon-ng/messaging/server.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <alcommon-ng/functor/makefunctor.hpp>
#include <allog/allog.h>

namespace AL {
  using namespace Messaging;
  namespace Common {

    ServerNodeImp::ServerNodeImp() {}

    ServerNodeImp::ServerNodeImp(
      const std::string& serverName,
      const std::string& serverAddress,
      const std::string& masterAddress) :
        fClient(masterAddress),
        AL::Messaging::Server(serverAddress)
     {
      fInfo.name = serverName;
      fInfo.address = serverAddress;
      setMessageHandler(this);
      boost::thread serverThread( boost::bind(&Server::run, this));
    }

    // shame about this definition of a handler....
    // would be great if we could do R onMessage( {mod, meth, T})
    boost::shared_ptr<AL::Messaging::ResultDefinition> ServerNodeImp::onMessage(const AL::Messaging::CallDefinition &def) {
      // handle message
      alsdebug << "  Server: " << fInfo.name << ", received message: " << def.methodName();

      std::string hash = def.methodName();
      const ServiceInfo& si = xGetService(hash);
      if (si.methodName.empty()) {
        // method not found
        alsdebug << "  Error: Method not found " << hash;
      }

      boost::shared_ptr<ResultDefinition> res = boost::shared_ptr<ResultDefinition>(
        new ResultDefinition());

      si.functor->call(def.args(), res->value());
      return res;
    }

    const NodeInfo& ServerNodeImp::getNodeInfo() const {
      return fInfo;
    }

    void ServerNodeImp::addService(const std::string& name, Functor* functor) {

      ServiceInfo service(name, functor);
      // We should be making a hash here, related to
      // "modulename.methodname" + typeid(arg0).name() ... typeid(argN).name()
      std::string hash = service.methodName;
      
      fLocalServiceList.insert(hash, service);
      xRegisterServiceWithMaster(hash);
    }

    const ServiceInfo& ServerNodeImp::xGetService(
      const std::string& methodHash) {
      // functors ... should be found here
      return fLocalServiceList.get(methodHash);
    }

    void ServerNodeImp::xRegisterServiceWithMaster(const std::string& methodHash) {
      if (fInfo.name != "master") {  // ehem
        CallDefinition callDef;
        callDef.methodName() = "master.registerService";
        callDef.args().push_back(fInfo.address);
        callDef.args().push_back(methodHash);

        fClient.send(callDef);
      }
    }
  }
}
