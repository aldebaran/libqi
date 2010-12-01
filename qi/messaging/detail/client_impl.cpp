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
#include <qi/messaging/detail/subscriber_impl.hpp>
#include <qi/log.hpp>

namespace qi {
  using serialization::SerializedData;

  namespace detail {

    ClientImpl::ClientImpl() {}

    ClientImpl::~ClientImpl() {
      if (_isInitialized) {
        xUnregisterSelfWithMaster();
      }
    }

    ClientImpl::ClientImpl(
      const std::string& clientName,
      const std::string& masterAddress) :
        ClientImplBase(masterAddress),
        _clientName(clientName)
    {
      _endpointContext.type = CLIENT_ENDPOINT;
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

      _isInitialized = _transportClient.connect(masterEndpointAndPort.first);
      if (! _isInitialized ) {
        qisError << "\"" << _clientName << "\" could not connect to master "
          "at address \"" << masterEndpointAndPort.first << "\""
          << std::endl;
        return;
      }
      xRegisterMachineWithMaster();
      xRegisterSelfWithMaster();
    }

    boost::shared_ptr<qi::transport::SubscribeHandlerUser> ClientImpl::subscribe(const std::string& topicName) {
      boost::shared_ptr<qi::detail::SubscriberImpl> subscriberImpl(new qi::detail::SubscriberImpl(_masterAddress));
      subscriberImpl->connect("tcp://127.0.0.1:6000");
      return subscriberImpl;
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

        std::string tmpAddress;
        try {
          tmpAddress = xLocateServiceWithMaster(methodSignature);
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
