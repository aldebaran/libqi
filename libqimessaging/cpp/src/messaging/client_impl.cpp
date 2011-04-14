/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/messaging/client_impl.hpp"
#include <qimessaging/exceptions.hpp>
#include <qi/log.hpp>

using qi::transport::TransportClient;

namespace qi {
  using serialization::Message;

  namespace detail {

    ClientImpl::~ClientImpl() {
      _masterClient.unregisterEndpoint(_endpointContext);
    }

    ClientImpl::ClientImpl(const std::string& name, Context *ctx)
      : ImplBase(ctx)
    {
      _endpointContext.type = CLIENT_ENDPOINT;
      _endpointContext.name = name;
      _endpointContext.contextID = _masterClient.getQiContextPtr()->getID();
    }

    void ClientImpl::connect(const std::string& masterAddress) {
      _masterClient.connect(masterAddress);
      _masterClient.registerMachine(_machineContext);
      _masterClient.registerEndpoint(_endpointContext);
      _isInitialized = _masterClient.isInitialized();
    }

    void ClientImpl::call(const std::string &signature,
      const qi::serialization::Message& callDef,
            qi::serialization::Message& result) {
        if (!_isInitialized) {
          throw( qi::transport::ConnectionException(
            "Initialization failed. All calls will fail."));
        }

        // will throw if service not found
        const std::string& serverEndpoint = xLocateService(signature);
        // will throw if unable to find or create client
        boost::shared_ptr<TransportClient> client = xGetServerClient(serverEndpoint);
        std::string resultData;
        client->send(callDef.str(), resultData);
        result.str(resultData);
    }

    boost::shared_ptr<TransportClient> ClientImpl::xGetServerClient(const std::string& serverEndpoint) {
      // get the relevant messaging client for the node that hosts the service

      boost::shared_ptr<TransportClient> tc = _serverClients.get(serverEndpoint);
      if (tc != NULL)
        return tc;

      // create messaging client if needed ...
      bool ok = xCreateServerClient(serverEndpoint);
      if (ok) {
        tc = _serverClients.get(serverEndpoint);
      }
      if (!ok || tc == NULL) {
        qisError << "Could not create client for server \"" << serverEndpoint
                 << "\" Probable connection error. " << std::endl;
        throw( qi::transport::ConnectionException(
          "Could not create client for server. Probable connection error."));
      }
      return tc;
    }

    bool ClientImpl::xCreateServerClient(const std::string& serverEndpoint) {
      boost::shared_ptr<TransportClient> client(new TransportClient(_masterClient.getQiContextPtr()->getTransportContext()));
      bool ok = client->connect(serverEndpoint);
      if (ok) {
        _serverClients.insert(serverEndpoint, client);
        qisDebug << "Client \"" << _endpointContext.name
                 << "\" connected to " << serverEndpoint << std::endl;
      }
      return ok;
    }

    const std::string& ClientImpl::xLocateService(const std::string& methodSignature) {
        const std::string& serverEndpoint = _serviceCache.get(methodSignature);
        if (!serverEndpoint.empty()) {
          return serverEndpoint;
        }

        std::string tmpEndpoint;
        try {
          tmpEndpoint = _masterClient.locateService(methodSignature, _endpointContext);
        } catch(const std::exception&) {
          qisWarning << "ServiceNotFoundException \"" << qi::signatureToString(methodSignature)
                     << "\" Unable to contact master." << std::endl;
          throw( qi::transport::ServiceNotFoundException(
            "Unable to contact master."));
        }

        if (!tmpEndpoint.empty()) {
          // cache located service pair
          _serviceCache.insert(methodSignature, tmpEndpoint);
          // return a const string ref address
          return _serviceCache.get(methodSignature);
        } else {
          qisWarning << "ServiceNotFoundException \"" << qi::signatureToString(methodSignature)
                     << "\" Method not known to master." << std::endl;
          throw( qi::transport::ServiceNotFoundException(
            "Method not known to master."));
        }

        // never happens: we either return early or throw
        return serverEndpoint;
    }
  }
}
