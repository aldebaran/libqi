/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/messaging/server_impl.hpp"
#include <string>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <qimessaging/serialization.hpp>
#include <qimessaging/transport/buffer.hpp>
#include "src/messaging/network/endpoints.hpp"
#include "src/messaging/network/master_endpoint.hpp"
#include <qi/log.hpp>
#include <qimessaging/exceptions.hpp>
#include <qimessaging/signature.hpp>

namespace qi {

  namespace detail {

    ServerImpl::~ServerImpl() {
      if (!_isMasterServer) {
        _masterClient.unregisterEndpoint(_endpointContext);
      }
    }

    ServerImpl::ServerImpl(const std::string& name, Context *ctx)
      : ImplBase(ctx),
        _isMasterServer(false),
        _transportServer()

    {
      _endpointContext.type = SERVER_ENDPOINT;
      _endpointContext.name = name;
      _endpointContext.contextID = _masterClient.getQiContextPtr()->getID();
    }

    void ServerImpl::connect(const std::string &masterAddress)
    {
      std::pair<std::string, int> masterEndpointAndPort;
      if (!qi::detail::validateMasterEndpoint(masterAddress, masterEndpointAndPort))
      {
        _isInitialized = false;
        qiLogError("qimessaging") << "\"" << _endpointContext.name << "\" initialized with invalid master "
          "address: \"" << masterAddress << "\" All calls will fail."
          << std::endl;
        return;
      }

      // TODO cleanup... the above work could be done in master client
      // all we need below is the port. Indeed, if coming from the master,
      // all we want is a port.

      if (_endpointContext.name == "master") {
        // we are the master's server, so we don't need a client to ourselves
        _isMasterServer = true;
        _isInitialized = true;
        _endpointContext.port = masterEndpointAndPort.second;
      } else {
        _masterClient.connect(masterAddress);
        _endpointContext.port = _masterClient.getNewPort(_endpointContext.machineID);
        _masterClient.registerMachine(_machineContext);
        _masterClient.registerEndpoint(_endpointContext);
        _isInitialized = _masterClient.isInitialized();
      }

      //CTAF: todo
      //_transportServer.serve(qi::detail::getEndpoints(_endpointContext, _machineContext));

      _transportServer.setDelegate(this);

      //CTAF: todo
      //boost::thread serverThread(
      //  ::boost::bind(&qi::TransportServer::run, &_transportServer));
    }

    void ServerImpl::messageHandler(std::string& defData, std::string& resultData) {
      // handle qi::DataStream
      qi::DataStream def(defData);
      qi::DataStream result(resultData);
      std::string methodSignature;
      def >> methodSignature;
      const ServiceInfo& si = xGetService(methodSignature);
      if (si.methodName.empty() || !si.functor) {
        qiLogError("qimessaging") << "Server Error: Method not found(" << methodSignature << "): " << qi::signatureToString(methodSignature) << std::endl;
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
        qiLogError("qimessaging") << "Attempt to use uninitialized server: \"" << _endpointContext.name <<
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
