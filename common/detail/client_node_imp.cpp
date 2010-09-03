/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/detail/client_node_imp.hpp>
#include <string>
#include <alcommon-ng/common/detail/get_protocol.hpp>
#include <alcommon-ng/messaging/client.hpp>
#include <allog/allog.h>
#include <alcommon-ng/exceptions/exceptions.hpp>

namespace AL {
  using namespace Messaging;
  namespace Common {

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
      initOK = xCreateServerClient(fMasterAddress);
      if (initOK) {
        // we assert that we think the master can locate services
        fServiceCache.insert("master.locateService::s#&:s", fMasterAddress);
      } else {
        alserror << "\"" << fClientName <<
          "\" Failed to connect to master at address \"" << fMasterAddress << "\"";
      }
    }

    void ClientNodeImp::call(
      const CallDefinition& callDef,  AL::Messaging::ResultDefinition &result) {
      if (! initOK) {
        throw( AL::Transport::ConnectionException(
          "Unable to find service. Reason: initial master connection failed."));
      }

      // will throw if service not found
      const std::string& nodeAddress = xLocateService(callDef.methodName());
      // will throw if unable to find or create client
      boost::shared_ptr<Client> client = xGetServerClient(nodeAddress);

      //TODO: optimise
      result = client->send(callDef);
      return;
    }

    // TODO const ref ?
    boost::shared_ptr<Client> ClientNodeImp::xGetServerClient(const std::string& serverAddress) {
       // get the relevant messaging client for the node that hosts the service
      NameLookup<boost::shared_ptr<Client> >::iterator it;
      it = fServerClients.find(serverAddress);
      if (it == fServerClients.end()) {
        // create messaging client if needed ...
        bool ok = xCreateServerClient(serverAddress);
        if (ok) {
          it = fServerClients.find(serverAddress);
        }
        if (!ok || it == fServerClients.end()) {
          alserror << "Could not create client for server \"" << serverAddress
            << "\" Probable connection error. ";
          throw( AL::Transport::ConnectionException(
            "Could not create client for server. Probable connection error."));
        }
      }
      return it->second;
    }

    bool ClientNodeImp::xCreateServerClient(const std::string& serverAddress) {
      // we don't yet know our current IP address, so we fake
      std::string serverFullAddress = getProtocol(serverAddress, serverAddress) + serverAddress;

      boost::shared_ptr<Client> client =
        boost::shared_ptr<Client>(new AL::Messaging::Client());
      bool ok = client->connect(serverFullAddress);
      if (ok) {
        fServerClients.insert(make_pair(serverAddress, client));
        alsdebug << "Client " << fClientName <<
          " creating client for server " << serverFullAddress << std::endl;
      }
      return ok;
    }

    const std::string& ClientNodeImp::xLocateService(
      const std::string& methodHash) {
      const std::string& nodeAddress = fServiceCache.get(methodHash);
      if (!nodeAddress.empty()) {
        return nodeAddress;
      }

      try {
        ResultDefinition r;
        call(CallDefinition("master.locateService::s#&:s", methodHash), r);
        std::string tmpAddress = r.value().as<std::string>();

        if (!tmpAddress.empty()) {
          // cache located service pair
          fServiceCache.insert(methodHash, tmpAddress);
          // return a const string ref address
          return fServiceCache.get(methodHash);
        } else {
          alserror << "Unable to find service \"" << methodHash
            << "\" Reason: Method not known to master.";
          throw( AL::Transport::ServiceNotFoundException(
            "Unable to find service. Reason: Method not known to master."));
        }
      } catch(const std::exception& e) {
        alserror << "Unable to find service \"" << methodHash
          << "\" Reason: master not found. Detail: " << e.what();
        throw( AL::Transport::ServiceNotFoundException(
          "Unable to find service. Reason: master not found."));
      }

      // never happens: we either return early or throw
      return nodeAddress;
    }
  }
}
