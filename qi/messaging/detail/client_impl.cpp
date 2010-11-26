/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/client_impl.hpp>
#include <string>
#include <qi/exceptions/exceptions.hpp>
#include <qi/transport/detail/network/master_endpoint.hpp>
#include <qi/log.hpp>


namespace qi {
  using serialization::SerializedData;

  namespace detail {

    ClientImpl::ClientImpl() : _isInitialized(false) {}

    ClientImpl::~ClientImpl() {
      if (_isInitialized) {
        xUnregisterSelfWithMaster();
      }
    }

    ClientImpl::ClientImpl(
      const std::string& clientName,
      const std::string& masterAddress) :
        _isInitialized(false),
        _clientName(clientName),
        _masterAddress(masterAddress)
    {
      _endpointContext.name = _clientName;
      _endpointContext.contextID = _qiContext.getID();
      xInit();
    }

    void ClientImpl::xInit() {
      std::pair<std::string, int> masterEndpointAndPort;
      if (!qi::detail::validateMasterEndpoint(_masterAddress, masterEndpointAndPort)) {
        _isInitialized = false;
        qisError << "\"" << _clientName << "\" initialized with invalid master address: \""
          << _masterAddress << "\" All calls will fail." << std::endl;
        return;
      }
      // create a messaging client to the master address
      _isInitialized = xCreateServerClient(masterEndpointAndPort.first);
      if (_isInitialized) {
        // we assert that we think the master can locate services
        // and that we can register and unregister ourselves
        _serviceCache.insert("master.registerClient::v:ssssi", masterEndpointAndPort.first);
        _serviceCache.insert("master.unregisterClient::v:s",   masterEndpointAndPort.first);
        _serviceCache.insert("master.locateService::s:ss",     masterEndpointAndPort.first);
        xRegisterSelfWithMaster();
      } else {
        qisError << "\"" << _clientName << "\" Failed to connect to master at address \""
                 << masterEndpointAndPort.first << "\"" << std::endl;
      }
    }

    bool ClientImpl::isInitialized() const {
      return _isInitialized;
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

      boost::shared_ptr<qi::transport::Client> client(new qi::transport::Client());
      bool ok = client->connect(serverAddress);
      if (ok) {
        _serverClients.insert(make_pair(serverAddress, client));
        qisDebug << "Client \"" << _clientName
                 << "\" connected to " << serverAddress << std::endl;
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
        request_msg.writeString("master.locateService::s:ss");
        request_msg.writeString(methodSignature);
        request_msg.writeString(_endpointContext.endpointID);
        std::string tmpAddress;

        try {
          call("master.locateService::s:ss", request_msg, response_msg);
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

    void ClientImpl::xRegisterSelfWithMaster() {
      static const std::string method = "master.registerClient::v:ssssi";
      SerializedData request_msg;
      SerializedData response_msg;
      request_msg.writeString(method);
      request_msg.writeString(_endpointContext.name);
      request_msg.writeString(_endpointContext.endpointID);
      request_msg.writeString(_endpointContext.contextID);
      request_msg.writeString(_endpointContext.machineID);
      request_msg.writeInt(_endpointContext.platformID);
      call(method, request_msg, response_msg);
    }

    void ClientImpl::xUnregisterSelfWithMaster() {
      static const std::string method = "master.unregisterClient::v:s";
      SerializedData request_msg;
      SerializedData response_msg;
      request_msg.writeString(method);
      request_msg.writeString(_endpointContext.endpointID);
      call(method, request_msg, response_msg);
    }
  }
}
