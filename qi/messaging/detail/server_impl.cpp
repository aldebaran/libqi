/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qi/messaging/detail/server_impl.hpp>
#include <string>
#include <qi/messaging/detail/get_protocol.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <qi/serialization/serializer.hpp>
#include <qi/transport/buffer.hpp>


namespace qi {
  using qi::serialization::SerializedData;

  namespace detail {

    ServerImpl::ServerImpl() : isInitialized(false) {}

    ServerImpl::~ServerImpl() {
      if (!fIsMasterServer) {
        xUnregisterSelfWithMaster();
      }
    }

    ServerImpl::ServerImpl(
      const std::string serverName,
      const std::string serverAddress,
      const std::string masterAddress) :
        isInitialized(false),
        fName(serverName),
        fAddress(serverAddress),
        fIsMasterServer(false)
    {
      if (serverAddress == masterAddress) {
        // we are the master's server, so we don't need a client to ourselves
        fIsMasterServer = true;
        isInitialized = true;
      } else {
        isInitialized = fMessagingClient.connect(getProtocol(serverAddress, masterAddress) + masterAddress);
        if (! isInitialized ) {
          qisError << "\"" << serverName << "\" could not connect to master at address \"" << masterAddress << "\"" << std::endl;
          return;
        }
        xRegisterSelfWithMaster();
      }

      fMessagingServer.serve("tcp://" + serverAddress);
      fMessagingServer.setMessageHandler(this);
      boost::thread serverThread(boost::bind(&qi::transport::Server::run, fMessagingServer));
    }

    void ServerImpl::messageHandler(std::string& defData, std::string& resultData) {
      // handle message
      SerializedData def(defData);
      SerializedData result(resultData);
      std::string methodSignature;
      def.read<std::string>(methodSignature);
      const ServiceInfo& si = xGetService(methodSignature);
      if (si.methodName.empty() || !si.functor) {
        qisError << "  Error: Method not found " << methodSignature << std::endl;
        return;
      }
      si.functor->call(def, result);
      resultData = result.str();
    }

    const std::string& ServerImpl::getName() const {
      return fName;
    }

    const std::string& ServerImpl::getAddress() const {
      return fAddress;
    }

    void ServerImpl::addService(const std::string& methodSignature, qi::Functor* functor) {
      ServiceInfo service(methodSignature, functor);
      //std::cout << "Added Service" << hash << std::endl;
      fLocalServiceList.insert(methodSignature, service);
      if (!fIsMasterServer) {
        xRegisterServiceWithMaster(methodSignature);
      }
    }

    const ServiceInfo& ServerImpl::xGetService(
      const std::string& methodSignature) {
      // functors ... should be found here
      return fLocalServiceList.get(methodSignature);
    }

    void ServerImpl::xRegisterServiceWithMaster(const std::string& methodSignature) {
      static const std::string method("master.registerService::v:ss");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer callDef;

      callDef.write<std::string>(method);
      callDef.write<std::string>(fAddress);
      callDef.write<std::string>(methodSignature);
      fMessagingClient.send(callDef.str(), ret);
    }

    void ServerImpl::xRegisterSelfWithMaster() {
      static const std::string method("master.registerServerNode::v:ss");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer callDef;

      callDef.write<std::string>(method);
      callDef.write<std::string>(fName);
      callDef.write<std::string>(fAddress);
      fMessagingClient.send(callDef.str(), ret);
    }

    void ServerImpl::xUnregisterSelfWithMaster() {
      static const std::string method("master.unregisterServerNode::v:ss");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer callDef;

      callDef.write<std::string>(method);
      callDef.write<std::string>(fName);
      callDef.write<std::string>(fAddress);
      fMessagingClient.send(callDef.str(), ret);
    }
  }
}
