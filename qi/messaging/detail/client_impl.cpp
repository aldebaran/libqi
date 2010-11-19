/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qi/messaging/detail/client_impl.hpp>
#include <string>
#include <qi/messaging/detail/get_protocol.hpp>
#include <qi/exceptions/exceptions.hpp>


namespace qi {
  using serialization::SerializedData;

  namespace detail {

    ClientImpl::ClientImpl() : _isInitialized(false) {}

    ClientImpl::~ClientImpl() {}

    ClientImpl::ClientImpl(
      const std::string& clientName,
      const std::string& masterAddress) :
        _isInitialized(false),
        _clientName(clientName),
        _masterAddress(masterAddress) {
        xInit();
    }

    void ClientImpl::xInit() {
      // create a messaging client to the master address
      _isInitialized = xCreateServerClient(_masterAddress);
      if (_isInitialized) {
        // we assert that we think the master can locate services
        _serviceCache.insert("master.locateService::s:s", _masterAddress);
      } else {
        qisError << "\"" << _clientName << "\" Failed to connect to master at address \""
                 << _masterAddress << "\"" << std::endl;
      }
    }

    void ClientImpl::call(const std::string &signature,
      const qi::serialization::SerializedData& callDef,
            qi::serialization::SerializedData &result) {
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
      NameLookup<boost::shared_ptr<qi::transport::Client> >::iterator it;
      it = _serverClients.find(serverAddress);
      if (it == _serverClients.end()) {
        // create messaging client if needed ...
        bool ok = xCreateServerClient(serverAddress);
        if (ok) {
          it = _serverClients.find(serverAddress);
        }
        if (!ok || it == _serverClients.end()) {
          qisError << "Could not create client for server \"" << serverAddress
                   << "\" Probable connection error. " << std::endl;
          throw( qi::transport::ConnectionException(
            "Could not create client for server. Probable connection error."));
        }
      }
      return it->second;
    }

    bool ClientImpl::xCreateServerClient(const std::string& serverAddress) {
      // we don't yet know our current IP address, so we fake
      std::string serverFullAddress = getProtocol(serverAddress, serverAddress) + serverAddress;

      boost::shared_ptr<qi::transport::Client> client(new qi::transport::Client());
      bool ok = client->connect(serverFullAddress);
      if (ok) {
        _serverClients.insert(make_pair(serverAddress, client));
        qisDebug << "Client " << _clientName
                 << " creating client for server " << serverFullAddress << std::endl;
      }
      return ok;
    }

    const std::string& ClientImpl::xLocateService(const std::string& methodSignature) {
        const std::string& nodeAddress = _serviceCache.get(methodSignature);
        if (!nodeAddress.empty()) {
          return nodeAddress;
        }

        SerializedData response_msg;

        // prepare a request to master.locateService
        SerializedData request_msg;
        request_msg.writeString("master.locateService::s:s");
        request_msg.writeString(methodSignature);
        std::string tmpAddress;

        try {
          call("master.locateService::s:s", request_msg, response_msg);
          response_msg.readString(tmpAddress);
        } catch(const std::exception&) {
          qisWarning << "ServiceNotFoundException \"" << methodSignature
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
          qisWarning << "ServiceNotFoundException \"" << methodSignature
                     << "\" Method not known to master." << std::endl;
          throw( qi::transport::ServiceNotFoundException(
            "Method not known to master."));
        }

        // never happens: we either return early or throw
        return nodeAddress;
    }
  }
}
