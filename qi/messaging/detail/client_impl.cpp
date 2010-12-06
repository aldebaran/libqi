/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/client_impl.hpp>
#include <qi/exceptions/exceptions.hpp>
#include <qi/transport/detail/network/master_endpoint.hpp>
#include <qi/log.hpp>

namespace qi {
  using serialization::Message;

  namespace detail {

    ClientImpl::ClientImpl() {}

    ClientImpl::~ClientImpl() {
      unregisterEndpoint(_endpointContext);
    }

    ClientImpl::ClientImpl(
      const std::string& clientName,
      const std::string& masterAddress) :
        MasterClient(clientName, masterAddress)
    {
      _endpointContext.type = CLIENT_ENDPOINT;
      init();
      registerMachine(_machineContext);
      registerEndpoint(_endpointContext);
    }

    void ClientImpl::call(const std::string &signature,
      const qi::serialization::Message& callDef,
            qi::serialization::Message &result) {
        if (!_isInitialized) {
          throw( qi::transport::ConnectionException(
            "Initialization failed. All calls will fail."));
        }

        // will throw if service not found
        const std::string& nodeAddress = xLocateService(signature);
        // will throw if unable to find or create client
        boost::shared_ptr<qi::transport::Client> client = xGetServerClient(nodeAddress);
        std::string resultData;
        client->send(callDef.str(), resultData);
        result.str(resultData);
    }

    boost::shared_ptr<qi::transport::Client> ClientImpl::xGetServerClient(const std::string& serverAddress) {
      // get the relevant messaging client for the node that hosts the service

      boost::shared_ptr<qi::transport::Client> c = _serverClients.get(serverAddress);
      if (c == NULL) {
        // create messaging client if needed ...
        bool ok = xCreateServerClient(serverAddress);
        if (ok) {
          c = _serverClients.get(serverAddress);
        }
        if (!ok || c == NULL) {
          qisError << "Could not create client for server \"" << serverAddress
                   << "\" Probable connection error. " << std::endl;
          throw( qi::transport::ConnectionException(
            "Could not create client for server. Probable connection error."));
        }
      }
      return c;
    }

    bool ClientImpl::xCreateServerClient(const std::string& serverAddress) {

      boost::shared_ptr<qi::transport::Client> client(new qi::transport::Client());
      bool ok = client->connect(serverAddress);
      if (ok) {
        _serverClients.insert(serverAddress, client);
        qisDebug << "Client \"" << _endpointContext.name
                 << "\" connected to " << serverAddress << std::endl;
      }
      return ok;
    }

    const std::string& ClientImpl::xLocateService(const std::string& methodSignature) {
        const std::string& nodeAddress = _serviceCache.get(methodSignature);
        if (!nodeAddress.empty()) {
          return nodeAddress;
        }

        std::string tmpAddress;
        try {
          tmpAddress = locateService(methodSignature, _endpointContext);
        } catch(const std::exception&) {
          qisWarning << "ServiceNotFoundException \"" << qi::signatureToString(methodSignature)
                     << "\" Unable to contact master." << std::endl;
          throw( qi::transport::ServiceNotFoundException(
            "Unable to contact master."));
        }

        if (!tmpAddress.empty()) {
          // cache located service pair
          _serviceCache.insert(methodSignature, tmpAddress);
          // return a const string ref address
          return _serviceCache.get(methodSignature);
        } else {
          qisWarning << "ServiceNotFoundException \"" << qi::signatureToString(methodSignature)
                     << "\" Method not known to master." << std::endl;
          throw( qi::transport::ServiceNotFoundException(
            "Method not known to master."));
        }

        // never happens: we either return early or throw
        return nodeAddress;
    }
  }
}
