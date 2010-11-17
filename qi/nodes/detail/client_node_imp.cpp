/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qi/nodes/detail/client_node_imp.hpp>
#include <string>
#include <qi/nodes/detail/get_protocol.hpp>
#include <qi/exceptions/exceptions.hpp>


namespace qi {
  using serialization::SerializedData;

  namespace detail {

    ClientNodeImp::ClientNodeImp() : initOK(false) {}

    ClientNodeImp::~ClientNodeImp() {}

    ClientNodeImp::ClientNodeImp(
      const std::string& clientName,
      const std::string& masterAddress) :
    initOK(false),
      fClientName(clientName),
      fMasterAddress(masterAddress) {
        xInit();
    }

    void ClientNodeImp::xInit() {
      // create a messaging client to the master address
      initOK = xCreateServerClient(fMasterAddress);
      if (initOK) {
        // we assert that we think the master can locate services
        fServiceCache.insert("master.locateService::s:s", fMasterAddress);
      } else {
        qisError << "\"" << fClientName << "\" Failed to connect to master at address \""
                 << fMasterAddress << "\"" << std::endl;
      }
    }

    void ClientNodeImp::call(const std::string &signature, const qi::serialization::SerializedData& callDef, qi::serialization::SerializedData &result) {
        if (! initOK) {
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

    boost::shared_ptr<qi::transport::Client> ClientNodeImp::xGetServerClient(const std::string& serverAddress) {
      // get the relevant messaging client for the node that hosts the service
      NameLookup<boost::shared_ptr<qi::transport::Client> >::iterator it;
      it = fServerClients.find(serverAddress);
      if (it == fServerClients.end()) {
        // create messaging client if needed ...
        bool ok = xCreateServerClient(serverAddress);
        if (ok) {
          it = fServerClients.find(serverAddress);
        }
        if (!ok || it == fServerClients.end()) {
          qisError << "Could not create client for server \"" << serverAddress
                   << "\" Probable connection error. " << std::endl;
          throw( qi::transport::ConnectionException(
            "Could not create client for server. Probable connection error."));
        }
      }
      return it->second;
    }

    bool ClientNodeImp::xCreateServerClient(const std::string& serverAddress) {
      // we don't yet know our current IP address, so we fake
      std::string serverFullAddress = getProtocol(serverAddress, serverAddress) + serverAddress;

      boost::shared_ptr<qi::transport::Client> client = boost::shared_ptr<qi::transport::Client>(new qi::transport::Client());
      bool ok = client->connect(serverFullAddress);
      if (ok) {
        fServerClients.insert(make_pair(serverAddress, client));
        qisDebug << "Client " << fClientName
                 << " creating client for server " << serverFullAddress << std::endl;
      }
      return ok;
    }

    const std::string& ClientNodeImp::xLocateService(const std::string& methodSignature) {
        const std::string& nodeAddress = fServiceCache.get(methodSignature);
        if (!nodeAddress.empty()) {
          return nodeAddress;
        }

        SerializedData r;
        SerializedData callDef;
        callDef.write<std::string>("master.locateService::s:s");
        callDef.write<std::string>(methodSignature);
        std::string tmpAddress;
        try {
          call("master.locateService::s:s", callDef, r);
          r.read<std::string>(tmpAddress);
        } catch(const std::exception&) {
          qisWarning << "ServiceNotFoundException \"" << methodSignature
                     << "\" Unable to contact master." << std::endl;
          throw( qi::transport::ServiceNotFoundException(
            "Unable to contact master."));
        }

        if (!tmpAddress.empty()) {
          // cache located service pair
          fServiceCache.insert(methodSignature, tmpAddress);
          // return a const string ref address
          return fServiceCache.get(methodSignature);
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
