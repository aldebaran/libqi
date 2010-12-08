/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/src/server_impl.hpp>
#include <string>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <qi/serialization/serializer.hpp>
#include <qi/transport/buffer.hpp>
#include <qi/messaging/src/network/endpoints.hpp>
#include <qi/messaging/src/network/master_endpoint.hpp>
#include <qi/log.hpp>
#include <qi/exceptions/exceptions.hpp>

namespace qi {
  using qi::serialization::Message;

  namespace detail {

    ServerImpl::~ServerImpl() {
      if (!_isMasterServer) {
        _masterClient.unregisterEndpoint(_endpointContext);
      }
    }

    ServerImpl::ServerImpl(const std::string name, Context *ctx)
      : ImplBase(ctx),
        _isMasterServer(false),
        _transportServer(_masterClient.getQiContextPtr()->getTransportContext())

    {
      _endpointContext.type = SERVER_ENDPOINT;
      _endpointContext.name = name;
      _endpointContext.contextID = _masterClient.getQiContextPtr()->getID();
    }

    void ServerImpl::connect(const std::string &masterAddress)
    {
      //TODO
      std::pair<std::string, int> masterEndpointAndPort;
      if (!qi::detail::validateMasterEndpoint(masterAddress, masterEndpointAndPort)) {
        _isInitialized = false;
        qisError << "\"" << _endpointContext.name << "\" initialized with invalid master "
          "address: \"" << masterAddress << "\" All calls will fail."
          << std::endl;
        return;
      }

      if (_endpointContext.name == "master") {
        // we are the master's server, so we don't need a client to ourselves
        _isMasterServer = true;
        _isInitialized = true;
        _endpointContext.port = masterEndpointAndPort.second;
      } else {
        _masterClient.connect(masterEndpointAndPort.first);
        _endpointContext.port = _masterClient.getNewPort(_endpointContext.machineID);
        _masterClient.registerMachine(_machineContext);
        _masterClient.registerEndpoint(_endpointContext);
      }

      _transportServer.serve(qi::detail::getEndpoints(_endpointContext, _machineContext));

      _transportServer.setMessageHandler(this);
      boost::thread serverThread(
        ::boost::bind(&qi::transport::TransportServer::run, _transportServer));
    }

    void ServerImpl::messageHandler(std::string& defData, std::string& resultData) {
      // handle message
      Message def(defData);
      Message result(resultData);
      std::string methodSignature;
      def.readString(methodSignature);
      const ServiceInfo& si = xGetService(methodSignature);
      if (si.methodName.empty() || !si.functor) {
        qisError << "Server Error: Method not found: " << qi::signatureToString(methodSignature) << std::endl;
        return;
      }
      si.functor->call(def, result);
      resultData = result.str();
    }

    void ServerImpl::advertiseService(
      const std::string& methodSignature,
      qi::Functor* functor)
    {
      if (! _isInitialized ) {
        qisError << "Attempt to use uninitialized server: \"" << _endpointContext.name <<
          "\". Service \"" << qi::signatureToString(methodSignature) << "\" not added."
          << std::endl;
        throw qi::transport::ServerException(
          std::string("Attempt to use uninitialized server: \"") +
          _endpointContext.name + "\". Service not added.");
        return;
      }
      ServiceInfo service(methodSignature, functor);
      //std::cout << "Added Service" << hash << std::endl;
      _localServices.insert(methodSignature, service);
      if (!_isMasterServer) {
        _masterClient.registerService(methodSignature, _endpointContext);
      }
    }

    void ServerImpl::unadvertiseService(const std::string& methodSignature) {
      _masterClient.unregisterService(methodSignature);
      _localServices.remove(methodSignature);
    }

    const ServiceInfo& ServerImpl::xGetService(
      const std::string& methodSignature)
    {
      // functors ... should be found here
      return _localServices.get(methodSignature);
    }
  }
}
