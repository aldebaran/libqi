/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/nodes/detail/server_node_imp.hpp>
#include <string>
#include <qi/nodes/detail/get_protocol.hpp>
#include <qi/messaging/server.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <allog/allog.h>

namespace qi {
  using namespace Messaging;
  namespace Nodes {

    ServerNodeImp::ServerNodeImp() : initOK(false) {}

    ServerNodeImp::~ServerNodeImp() {
      if (!fIsMasterServer) {
        xUnregisterSelfWithMaster();
      }
    }

    ServerNodeImp::ServerNodeImp(
      const std::string serverName,
      const std::string serverAddress,
      const std::string masterAddress) :
        initOK(false),
        fName(serverName),
        fAddress(serverAddress),
        fIsMasterServer(false)
    {
      if (serverAddress == masterAddress) {
        // we are the master's server, so we don't need a client to ourselves
        fIsMasterServer = true;
        initOK = true;
      } else {
        initOK = fMessagingClient.connect(getProtocol(serverAddress, masterAddress) + masterAddress);
        if (! initOK ) {
          alserror << "\"" << serverName << "\" could not connect to master at address \"" << masterAddress << "\"";
          return;
        }
        xRegisterSelfWithMaster();
      }

      fMessagingServer.serve("tcp://" + serverAddress);
      fMessagingServer.setMessageHandler(this);
      boost::thread serverThread(boost::bind(&Server::run, fMessagingServer));
    }

    void ServerNodeImp::messageHandler(const CallDefinition& def, ResultDefinition& result) {
      // handle message
      std::string methodSignature = def.methodName();
      const ServiceInfo& si = xGetService(methodSignature);
      if (si.methodName.empty() || !si.functor) {
        alserror << "  Error: Method not found " << methodSignature;
      }
      si.functor->call(def.args(), result.value());
    }

    const std::string& ServerNodeImp::getName() const {
      return fName;
    }

    const std::string& ServerNodeImp::getAddress() const {
      return fAddress;
    }

    void ServerNodeImp::addService(const std::string& methodSignature, qi::Functor* functor) {
      ServiceInfo service(methodSignature, functor);
      //std::cout << "Added Service" << hash << std::endl;
      fLocalServiceList.insert(methodSignature, service);
      if (!fIsMasterServer) {
        xRegisterServiceWithMaster(methodSignature);
      }
    }

    const ServiceInfo& ServerNodeImp::xGetService(
      const std::string& methodSignature) {
      // functors ... should be found here
      return fLocalServiceList.get(methodSignature);
    }

    void ServerNodeImp::xRegisterServiceWithMaster(const std::string& methodSignature) {
      ResultDefinition r;
      fMessagingClient.call(CallDefinition("master.registerService::v:ss", fAddress, methodSignature), r);
    }

    void ServerNodeImp::xRegisterSelfWithMaster() {
      ResultDefinition r;
      fMessagingClient.call(CallDefinition("master.registerServerNode::v:ss", fName, fAddress), r);
    }

    void ServerNodeImp::xUnregisterSelfWithMaster() {
      ResultDefinition r;
      fMessagingClient.call(CallDefinition("master.unregisterServerNode::v:ss", fName, fAddress), r);
    }
  }
}
