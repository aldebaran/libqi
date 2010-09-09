/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/detail/server_node_imp.hpp>
#include <string>
#include <alcommon-ng/common/detail/get_protocol.hpp>
#include <alcommon-ng/messaging/server.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <allog/allog.h>

namespace AL {
  using namespace Messaging;
  namespace Common {

    ServerNodeImp::ServerNodeImp() : initOK(false) {}

    ServerNodeImp::~ServerNodeImp() {}

    ServerNodeImp::ServerNodeImp(
      const std::string& serverName,
      const std::string& serverAddress,
      const std::string& masterAddress) : initOK(false)
    {
      fInfo.name = serverName;
      fInfo.address = serverAddress;

      bool initOK = fClient.connect(getProtocol(serverAddress, masterAddress) + masterAddress);
      if (! initOK ) {
        alserror << "\"" << serverName << "\" could not connect to master at address \"" << masterAddress << "\"";
        return;
      }
      fServer.serve("tcp://" + serverAddress);

      fServer.setMessageHandler(this);
      boost::thread serverThread(boost::bind(&Server::run, fServer));
    }

    void ServerNodeImp::messageHandler(const CallDefinition &def, ResultDefinition& result) {
      // handle message
      std::string hash = def.methodName();
      const ServiceInfo& si = xGetService(hash);
      if (si.methodName.empty() || !si.functor) {
        // method not found
        alserror << "  Error: Method not found " << hash;
      }
      si.functor->call(def.args(), result.value());
    }

    const NodeInfo& ServerNodeImp::getNodeInfo() const {
      return fInfo;
    }

    void ServerNodeImp::addService(const std::string& name, Functor* functor) {
      ServiceInfo service(name, functor);
      std::string hash = service.methodName;
      //std::cout << "Added Service" << hash << std::endl;
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
        ResultDefinition r;
        fClient.call(CallDefinition("master.registerService::v:ss", fInfo.address, methodHash),r);
      }
    }
  }
}
