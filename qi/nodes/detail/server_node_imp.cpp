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
#include <qi/log.hpp>
#include <qi/serialization/serializeddata.hpp>

namespace qi {
  using namespace messaging;
  using qi::serialization::SerializedData;

  namespace detail {

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
          qisError << "\"" << serverName << "\" could not connect to master at address \"" << masterAddress << "\"" << std::endl;
          return;
        }
        xRegisterSelfWithMaster();
      }

      fMessagingServer.serve("tcp://" + serverAddress);
      fMessagingServer.setMessageHandler(this);
      boost::thread serverThread(boost::bind(&Server::run, fMessagingServer));
    }

    void ServerNodeImp::messageHandler(const std::string& defData, std::string& resultData) {
      // handle message
      SerializedData def(defData);
      SerializedData result(resultData);
      std::string methodSignature;
      def.read<std::string>(methodSignature);

      const ServiceInfo& si = xGetService(methodSignature);
      if (si.methodName.empty() || !si.functor) {
        qisError << "  Error: Method not found " << methodSignature << std::endl;
      }
      si.functor->call(def, result);
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
      static const std::string method("master.registerService::v:ss");
      std::string              ret;
      SerializedData           callDef;

      callDef.write<std::string>(method);
      callDef.write<std::string>(fName);
      callDef.write<std::string>(methodSignature);
      fMessagingClient.call(callDef.str(), ret);
    }

    void ServerNodeImp::xRegisterSelfWithMaster() {
      static const std::string method("master.registerServerNode::v:ss");
      std::string              ret;
      SerializedData           callDef;

      callDef.write<std::string>(method);
      callDef.write<std::string>(fName);
      callDef.write<std::string>(fAddress);
      fMessagingClient.call(callDef.str(), ret);
    }

    void ServerNodeImp::xUnregisterSelfWithMaster() {
      static const std::string method("master.unregisterServerNode::v:ss");
      std::string              ret;
      SerializedData           callDef;

      callDef.write<std::string>(method);
      callDef.write<std::string>(fName);
      callDef.write<std::string>(fAddress);
      fMessagingClient.call(callDef.str(), ret);
    }
  }
}
